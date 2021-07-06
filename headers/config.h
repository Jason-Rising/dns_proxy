#ifndef CONFIG_H
#define CONFIG_H

typedef struct config_t Config;

// Struct holding information for connecting to external DNS server
struct config_t
{
    char* IPv4;
    int port;
};

// Prints available arguments if invalid command arguments recieved
void print_usage();

// Retrieve program configurations from command arguments
Config* init(int argc, char** argv);

#endif