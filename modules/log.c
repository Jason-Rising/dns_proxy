#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../headers/log.h"
#include "../headers/dns.model.h"
#include "../headers/cache.h"

// Constants
#define FILE_NAME "dns_svr.log"

#define QUERY 0

// Log templates
#define T_BUFF_S 80
#define TIME_FORMAT "%FT%T%z"
#define REQUEST "%s requested %s\n"
#define INVALID_REQUEST "%s unimplemented request\n"
#define RESPONSE "%s %s is at %s\n"
#define IN_CACHE "%s %s expires at %s\n"
#define EVICTED_C "%s replacing %s by %s\n"

// Write line to log file
void log_query(Query* query){
    FILE *log = fopen(FILE_NAME, "a+");

    char time_stamp[T_BUFF_S];
    if (query->header->QR == QUERY){
        // Log request
        fprintf(log, REQUEST, get_time(time_stamp), query->question->QNAME);
        fflush(log);

        // Check if invalid ip type
        if (query->question->QTYPE != IPv6){
            fprintf(log, INVALID_REQUEST, get_time(time_stamp));
            fflush(log);
        }
    }else{
        // Is response
        // Check that there is at least one answer and answer is AAAA
        if (query->answer->A_RR > 0){
            fprintf(log, RESPONSE, get_time(time_stamp), query->question->QNAME, query->answer->RDDATA);
            fflush(log); 
        }
    }

    fclose(log);
}

// Returns current timestamp
char* get_time(char* t_buffer){
    time_t raw_time;
    struct tm* info;

    time(&raw_time);
    info = localtime(&raw_time);
    strftime(t_buffer, T_BUFF_S, TIME_FORMAT, info);
    return t_buffer;
}

// Return time n seconds into the future
char* time_append(time_t original_time, int seconds, char* t_buffer){
    time_t new_time = original_time + seconds;
    struct tm* info;
    info = localtime(&new_time);
    strftime(t_buffer, T_BUFF_S, TIME_FORMAT, info);
    return t_buffer;
}

// Write cache evication line to log
void log_cache_eviction(CacheEntry* new_entry, CacheEntry* old_entry){
    FILE *log = fopen(FILE_NAME, "a+");
    char time_stamp[T_BUFF_S];
    fprintf(log, EVICTED_C, get_time(time_stamp), old_entry->domain, new_entry->domain);
    fclose(log);   
}

// Write cache request line to log
void log_cache_request(CacheEntry* entry){
    FILE *log = fopen(FILE_NAME, "a+");
    char time_stamp[T_BUFF_S];
    char buffer[T_BUFF_S];
    fprintf(log, IN_CACHE, get_time(time_stamp), entry->domain, time_append(entry->start_time, entry->original_TTL, buffer));
    fclose(log);
}