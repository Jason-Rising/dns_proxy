#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../headers/dns.model.h"
#include "../headers/util.h"

// Constants
#define RESPONSE 1
#define IPv6_D 28

// Indicates positions were to access query sections
#define H_ID_PTR 2
#define H_FLAGS_PTR 4
#define H_ANSWER_RR 8
#define QNAME_PTR 14
// No. of bytes from end of QNAME
#define ANSWER_PTR_OFFSET 16
#define ANSWER_PTR_OFFSET_T 18

// Create and return an empty DNS query
Query* create_dns_query(){

    Query* query = (Query*)malloc(sizeof(*query));
    assert(query);

    query->header = (Header*)malloc(sizeof(query->header));
    assert(query->header);
    
    query->question = (Question*)malloc(sizeof(query->question));
    assert(query->question);

    query->answer = (Answer*)malloc(sizeof(query->answer));
    assert(query->answer);

    query->raw_request = (Byte_Array*)malloc(sizeof(query->raw_request));
    assert(query->raw_request);

    query->raw_repsonse = (Byte_Array*)malloc(sizeof(query->raw_repsonse));
    assert(query->raw_repsonse);

    query->header->QR = -1;
    
    return query;
}

// Free memory of dns struct
void free_dns_query(Query* query){
    // Free raw byte arrays
    free(query->raw_request->data);
    free(query->raw_request);

    // If not a response raw_res data was never malloced no need to free
    if (query->header->QR == RESPONSE) free(query->raw_repsonse->data);
    free(query->raw_repsonse);
    
    // Free header
    free(query->header);

    // Free question components
    free(query->question->QNAME);
    free(query->question);

    // Free answer components
    free(query->answer->RDDATA);
    free(query->answer);
    free(query);
}

// Updates query based on raw data in either question or answer fields
void update_query(Query* query){

    if (query->header->QR == -1){
        // Update from question section required
        query->header->QR = __get_QR(query->raw_request->data);
        query->question->QNAME = __get_QNAME(query->raw_request->data);
        query->question->QTYPE = __get_QTYPE(query->raw_request->data);
    }

    if (query->header->QR == RESPONSE){
        // Check that there is at least one answer
        query->answer->A_RR = __get_ANSWERRR(query->raw_repsonse->data);

        if (__get_ANSWER_TYPE(query->raw_repsonse->data) != 28){
            // First response is not AAAA set A_RR to -1 indicating not valid
            query->answer->A_RR = -1;
        }

        if (query->answer->A_RR > 0){
            // We can extract RDDATA
            query->answer->RDDATA = __get_RDDATE(query->raw_repsonse->data);
        }
    }

}

// Return QR value given raw packet data
int __get_QR(char* raw){
    char flagsUpperNibble = raw[H_FLAGS_PTR];
    return (flagsUpperNibble >> 7) & 0x1;
}

// Return QNAME given raw packet data
char* __get_QNAME(char* raw){
    int i = QNAME_PTR, j = 0;
    char *qname = (char*)malloc(sizeof(qname) *  __get_QNAME_len(raw));    
    assert(qname);

    int section_len = 0;
    while(raw[i] != 0){
        if (section_len == 0){
            // We're at the start of a new section; get size
            section_len = (short)(raw[i]);
            if (j != 0) qname[j++] = '.';
            
        }else {
            section_len--;
            qname[j++] = raw[i];
        }

        i++;
    }
    qname[j] = '\0';
    return qname;  
}

// Returns length of QNAME
int __get_QNAME_len(char *raw){
    int i = QNAME_PTR, count = 0;
    while(raw[i] != 0){
        count++;
        i++;
    }
    return count;
}


// Return IP type given raw packet data
IP_t __get_QTYPE(char *raw){
    // + 1 for 0x00 not added to count in __get_QNAME_len
    int QTYPE_PTR =  QNAME_PTR + __get_QNAME_len(raw) + 1;
    short type = concat_bytes(raw[QTYPE_PTR], raw[QTYPE_PTR + 1]);

    if (type == IPv6_D){
        return IPv6;   
    }
    return IPv4;
}

// Returns number of anwsers in response given raw packet data
short __get_ANSWERRR(char *raw){
    return concat_bytes(raw[H_ANSWER_RR], raw[H_ANSWER_RR + 1]);
}

// Return RDDATA (text version) given raw packet data
char* __get_RDDATE(char *raw){
    int ANSWER_PTR = (QNAME_PTR + __get_QNAME_len(raw) + 1) + ANSWER_PTR_OFFSET;
    
    // Convert binary to text
    char *ipv6_address = (char*)malloc(sizeof(ipv6_address) * INET6_ADDRSTRLEN);
    assert(ipv6_address);

    inet_ntop(AF_INET6, &raw[ANSWER_PTR], ipv6_address, INET6_ADDRSTRLEN);
    return ipv6_address;
}

// Get Answer type
short __get_ANSWER_TYPE(char *raw){
    int ANSWER_PTR = (QNAME_PTR + __get_QNAME_len(raw) + 1) + 6;
    return concat_bytes(raw[ANSWER_PTR], raw[ANSWER_PTR + 1]);
}