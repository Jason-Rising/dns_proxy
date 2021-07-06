#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

#include "headers/config.h"
#include "headers/dns.model.h"
#include "headers/socket.h"
#include "headers/log.h"
#include "headers/cache.h"
#include "headers/util.h"

// Specifying cache is implemented 
#define CACHE

// Constants:
#define PORT 8053
#define N_LIS_Q 10

// Run DNS proxy
void run_proxy(Config* resolver_server);

// Closes client connection if request is not AAAA
int validate_QTYPE(int connfd, Query* query);


int main(int argc, char** argv){
    // Get command argument values
    Config* resolver_server = init(argc, argv);
    run_proxy(resolver_server);

    return 0;
}

// Run DNS proxy
void run_proxy(Config* resolver_server){
    int listenfd = create_server_socket(PORT), connfd, nread;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;

    // Init cache
    ListCache* cache = create_empty_list();

    // Listen on socket
    if (listen(listenfd, N_LIS_Q) != 0){
        perror("Error: Failed to listen\n");
        exit(EXIT_FAILURE);
    }else fprintf(stderr, "Server listerning on port %d...\n\n", PORT);

    // Accept client connect requests and process messages
    while ((connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_size)) > 0){
        puts("Server got connection...");

        // Init query and recieve request
        Query* query = create_dns_query();

        read_to_byte_array(connfd, query->raw_request);
        update_query(query);

        // Log request
        log_query(query);

        // Check is valid QTYPE
        if (!validate_QTYPE(connfd, query)) continue;

        // Check if query cached
        if (cache_lookup(cache, query) == false){
            // Forward query to resolver and get response
            get_query_response(resolver_server->port, resolver_server->IPv4, query);
            update_query(query);
            if (query->answer->A_RR > 0){
                // Add response to cache
                insert_cache(cache, query);
            }
        }

        // Log response
        log_query(query);

        // Return resolver response to client
        nread =  write(connfd, query->raw_repsonse->data, query->raw_repsonse->n);
        printf("Writing %d bytes to client...\n", nread);


        // Close client connection
        close(connfd);
        puts("Server closed client connection...\n\n");
        free_dns_query(query);
    }

    free_list(cache);
}

// Closes client connection if request is not AAAA
int validate_QTYPE(int connfd, Query* query){
    assert(query);
    if (query->question->QTYPE != IPv6){
        // Invalid QTYPE send back Rcode 4
        respond_rcode4(connfd, query);
        
        // Close client connection
        close(connfd);
        free_dns_query(query);
        puts("Server closed client connection due to invalid QTYPE...\n\n");
        return false;
    }
    return true;
}