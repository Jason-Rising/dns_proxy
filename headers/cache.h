#ifndef CACHE_h
#define CACHE_h

#include "dns.model.h"

#define CACHE_N 5

typedef struct cache_t CacheEntry;

// Holds information regarding query in cache
struct cache_t {
    time_t start_time;
    int original_TTL;
    int TTL;
    char *domain;
    char *IPv6;
    char* raw_response;
    int n;
    int expired;

    CacheEntry* next;
    CacheEntry* prev;
};

typedef struct {
    CacheEntry* head;
    CacheEntry* foot;
    int n;

} ListCache;

// Create new node
CacheEntry* __create_new_node(Query* query);

// Init empty cache list
ListCache* create_empty_list();

// Insert new node at foot of list
void insert_cache(ListCache* list, Query* query);

// Check cache for domain
int cache_lookup(ListCache* list, Query* request);

// Update query with cache response
void __set_query_response(Query* query, CacheEntry* entry);

// Update TTL values of cache and remove expired entries
void __update_cache(ListCache* list);

// Update raw response with new TTL
void __set_raw_TTL(CacheEntry* entry, int new_TTL);

// Remove cache entry from list
void __remove_cache_entry(ListCache* list, CacheEntry* entry);

// Return TTL of given response
int __get_TTL(Query* response);

// Free list memory
void free_list(ListCache* list);

// Return first expired entry is any. Return NULL otherwise
CacheEntry* __find_expired(ListCache* list);

#endif