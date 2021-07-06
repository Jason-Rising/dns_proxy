#ifndef LOG_H
#define LOG_H

#include "dns.model.h"
#include "cache.h"

// Write line to log file
void log_query(Query* query);

// Returns current timestamp
char* get_time(char* t_buffer);

// Return time n seconds into the future
char* time_append(time_t original_time, int seconds, char* t_buffer);

// Write cache evication line to log
void log_cache_eviction(CacheEntry* new_entry, CacheEntry* old_entry);

// Write cache request line to log
void log_cache_request(CacheEntry* entry);

#endif
