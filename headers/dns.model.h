#ifndef DNS_MODEL_H
#define DNS_MODEL_H

typedef struct dns_header_t Header;
typedef struct dns_question_t Question;
typedef struct dns_answer_t Answer;
typedef struct dns_raw_t Byte_Array;

typedef struct dns_query_t Query;

// Represents differnt IP types
typedef enum
{
    IPv4,
    IPv6
}IP_t;

// Holds DNS header info
struct dns_header_t
{
    short QR;
};

// Holds DNS question information
struct dns_question_t {
    char* QNAME;
    IP_t QTYPE;
};

// Holds DNS answer information
struct dns_answer_t {
    short A_RR;
    char* RDDATA;
};

// Holds raw binary response or request 
struct dns_raw_t
{
    char* data;
    int n;
};

// Struct representing DNS query
struct dns_query_t
{
    Header* header;
    Question* question;
    Answer* answer;

    // Raw bytes
    Byte_Array* raw_request;
    Byte_Array* raw_repsonse;
};

// Create and return an empty DNS query
Query* create_dns_query();

// Free memory of dns struct
void free_dns_query(Query* query);

// Updates query based on raw data in either question or answer fields
void update_query(Query* query);

// Return QR value given raw packet data
int __get_QR(char* raw);

// Return QNAME given raw packet data
char* __get_QNAME(char* raw);

// Returns length of QNAME
int __get_QNAME_len(char *raw);

// Return QTYPE given raw packet data
IP_t __get_QTYPE(char *raw);

// Returns number of anwsers in response given raw packet data
short __get_ANSWERRR(char *raw);

// Return RDDATA (text version) given raw packet data
char* __get_RDDATE(char *raw);

// Get Answer type
short __get_ANSWER_TYPE(char *raw);

#endif