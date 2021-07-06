#ifndef SOCKET_H
#define SOCKET_H

#include "dns.model.h"

// Create and return socket bound to specificed port
int create_server_socket(const int port);

// Create and return socket bound to specificed port and server
int create_client_socekt(const int port, const char* server_name, struct sockaddr_in* serv_addr);

// Creates connection with resolver server and sends given DNS query. 
void get_query_response(const int port, const char* ipv4, Query* dns_query);

// Read from socket and dump entire message into custom Byte_Array struct
void read_to_byte_array(int socket, Byte_Array* raw);

// Send Rcode 4 to client 
void respond_rcode4(int socket, Query* query);

// Updates byte to be rdcode4
char __set_rcode4(char byte);

#endif
