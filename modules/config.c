#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../headers/config.h"

// Prints available arguments if invalid command arguments recieved
void print_usage(){
    printf("Arguments: <dns server ipv4> | <dns server port> \n");
}

// Retrieve program configurations from command arguments
Config* init(int argc, char** argv){
    if (argc != 3){
        print_usage();
        exit(EXIT_FAILURE);
    }

    // Retrieve command arguments
    Config* config = (Config*)malloc(sizeof(*config));
    assert(config);

    config->IPv4 = argv[1];
    config->port= atoi(argv[2]);

    return config;
}
