#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "../headers/util.h"
#include "../headers/dns.model.h"

// Constants
#define BUFF_SIZE 16
#define RESPONSE 1
#define H_FLAGS_PTR 4

#define RCODE_PTR 5

// Create and return socket bound to specificed port
int create_server_socket(const int port){
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("Error: Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

   // Reuse port if possible
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("Error: Failed to set socket opt\n");
        exit(EXIT_FAILURE);
    }

    // Initialise server address and send buffer
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serv_addr.sin_port = htons(port);  

    // Bind & Listen
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        perror("Error: Failed to bind\n");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

// Create and return socket bound to specificed port and server
int create_client_socekt(const int port, const char* ipv4, struct sockaddr_in* serv_addr){
	int sockfd;
	struct hostent* server;

	server = gethostbyname(ipv4);

	if (!server) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(EXIT_FAILURE);
	}

	memset((char*)serv_addr, 0, sizeof(serv_addr));

	serv_addr->sin_family = AF_INET;
    memcpy((char*)&serv_addr->sin_addr.s_addr, server->h_addr_list[0], server->h_length);
	serv_addr->sin_port = htons(port);

	// Create socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Error: Failed to create client socket");
		exit(EXIT_FAILURE);
	}

	return sockfd;
}

// Read from socket and dump entire message into custom Byte_Array struct
void read_to_byte_array(int socket, Byte_Array* raw){
    assert(raw);
    char buffer[BUFF_SIZE];
    int n, i, j = 0;
    raw->n = 0;
    
    do {
        n = recv(socket, buffer, sizeof(buffer), 0);
        if (n < 0){
            // Error occured
            perror("Error: receiving message\n");
            exit(EXIT_FAILURE);
        }
        
        if (raw->n == 0){
            // Recieving first bytes extract packet size and malloc. + 2 to include 2 byte header
            raw->n = concat_bytes(buffer[0], buffer[1]) + 2;
            raw->data = (char*)malloc(sizeof(raw->data) *  raw->n);       
            assert(raw->data); 
            printf("Recieving message of %d bytes...\nMessage (Hex):\n", raw->n);           
        }


        // Append buffer contents to raw
        for (i = 0; i < n; i++){
            // Print HEX representation of message
            print_hex(buffer[i]);

            raw->data[j++] = buffer[i];
        }
        puts("");

    } while (j < raw->n);
}

// Creates connection with resolver server and sends given DNS query. Returns entire resolver response
void get_query_response(const int port, const char* ipv4, Query* dns_query){
    struct sockaddr_in serv_addr;
    int sockfd = create_client_socekt(port, ipv4, &serv_addr);

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error: Failed to connect to resolver server\n");
        exit(EXIT_FAILURE);
    }  

    //Send Query to resolver
    int n_w;
    if ((n_w = write(sockfd, dns_query->raw_request->data, dns_query->raw_request->n)) < 0){
        perror("Error: Failed to send query to resolver\n");
        exit(EXIT_FAILURE);
    } else printf("Writting %d bytes to resolver...\n", n_w);

    // Read response from resolver and write to query struct
    read_to_byte_array(sockfd, dns_query->raw_repsonse);
    
    // Update QR
    dns_query->header->QR = RESPONSE;

    // Close connection
    close(sockfd);
}

// Updates byte to be rdcode4
char __set_rcode4(char byte){
    byte = (byte & 0xF0);
    return byte |= (1 << 2);
}

// Send Rcode 4 to client 
void respond_rcode4(int socket, Query* query){
    // Create response
    query->raw_repsonse->data = query->raw_request->data;
    query->raw_repsonse->n = query->raw_request->n;

    // Update lower nibble to 4 (00100)
    query->raw_repsonse->data[RCODE_PTR] = __set_rcode4(query->raw_repsonse->data[RCODE_PTR]);

    // Set QR to response
    query->raw_repsonse->data[H_FLAGS_PTR] |= (1 << 7);

    // Set RA to 1
    query->raw_repsonse->data[H_FLAGS_PTR + 1] |= (1 << 7);

    int nread;
    nread =  write(socket, query->raw_repsonse->data, query->raw_repsonse->n);
    printf("Writing %d bytes to client...\n", nread);
}