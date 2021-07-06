#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "../headers/cache.h"
#include "../headers/dns.model.h"
#include "../headers/util.h"
#include "../headers/log.h"

#define ID_PTR 2
#define QNAME_PTR 14
#define TTL_PTR_OFFSET 11

// Create new node
CacheEntry* __create_new_node(Query* query){
    CacheEntry* new_entry = (CacheEntry*)malloc(sizeof(*new_entry));
    assert(new_entry);

    time_t start;
    time(&start);

    new_entry->next = NULL;
    new_entry->prev = NULL;

    new_entry->start_time = start;
    new_entry->TTL = __get_TTL(query);
    new_entry->original_TTL = new_entry->TTL;
    new_entry->n = query->raw_repsonse->n;
    new_entry->expired = false;

    // Copy over domain, raw response and IPv6
    // Domain
    new_entry->domain = (char*)malloc(sizeof(new_entry->domain) * strlen(query->question->QNAME));
    assert(new_entry->domain);
    strcpy(new_entry->domain, query->question->QNAME);

    // Raw response
    new_entry->raw_response = (char*)malloc(sizeof(new_entry->raw_response) * query->raw_repsonse->n);
    assert(new_entry->raw_response);
    memcpy(new_entry->raw_response, query->raw_repsonse->data, query->raw_repsonse->n);
    new_entry->IPv6 = __get_RDDATE(query->raw_repsonse->data);
    return new_entry;
}

// Init empty cache list
ListCache* create_empty_list(){
    ListCache* list = (ListCache*)malloc(sizeof(*list));
    assert(list);

    list->head = list->foot = NULL;
    list->n = 0;
    return list;
}

// Insert new node at foot of list
void insert_cache(ListCache* list, Query* query){
    CacheEntry* new = __create_new_node(query);
    assert(new && list);
    if (list->n >= CACHE_N){
        // Replace at head
        printf("Replacing cache entry %s with %s, TTL: %d...\n", list->head->domain, new->domain, new->TTL);
        log_cache_eviction(new, list->head);
        list->head->prev = new;
        new->next = list->head;
        list->head = new;
    }else{
        // Check for expired 
        CacheEntry* expired = __find_expired(list);
        if (expired != NULL){
            // Replace expired
            printf("Replacing cache entry %s with %s, TTL: %d...\n", list->head->domain, new->domain, new->TTL);
            log_cache_eviction(new, expired);
            __remove_cache_entry(list, expired);
        }else printf("Inserting new cache entry for %s with TTL: %d...\n", new->domain, new->TTL);

        // Add to foot
        if (list->foot == NULL){
            list->head = list->foot = new;
        }else{
            new->prev = list->foot;
            list->foot->next = new;
            list->foot = new;
        }    
    }
    list->n += 1;
} 

// Return first expired entry is any. Return NULL otherwise
CacheEntry* __find_expired(ListCache* list){
    CacheEntry* curr = list->head;
    while(curr){
        if (curr->expired == true) return curr;
        curr = curr->next;
    }    
    return NULL;
}

// Check cache for domain
int cache_lookup(ListCache* list, Query* request){
    assert(list);
    __update_cache(list);
    
    CacheEntry* curr = list->head;
    while(curr){
        if (strcmp(curr->domain, request->question->QNAME) == 0 && curr->expired == false){
            // Found match update query response 
            printf("Found %s in cache...\n", curr->domain);
            log_cache_request(curr);
            __set_query_response(request, curr);
            return 1;
        }

        curr = curr->next;
    }
    return 0;
}

// Update TTL values of cache and remove expired entries
void __update_cache(ListCache* list){
    assert(list);
    CacheEntry* curr = list->head;
    time_t curr_time;
    time(&curr_time);
    int new_TTL;
    while(curr){
        new_TTL = curr->TTL;
        new_TTL -= (int)difftime(curr_time, curr->start_time);
        if (new_TTL < 0){
            // Entry expired
            curr->expired = true;

        }else{
            // Update TTL 
            curr->TTL = new_TTL;
            __set_raw_TTL(curr, new_TTL);
        }

        curr = curr->next;
    }    
}

// Update raw response TTL with new TTL
void __set_raw_TTL(CacheEntry* entry, int new_TTL){
    char* split_TTL = split_32_t_bytes(new_TTL);
    int TTL_ptr = __get_QNAME_len(entry->raw_response) + QNAME_PTR + TTL_PTR_OFFSET;
    entry->raw_response[TTL_ptr] = split_TTL[0];
    entry->raw_response[TTL_ptr + 1] = split_TTL[1];
    entry->raw_response[TTL_ptr + 2] = split_TTL[2];
    entry->raw_response[TTL_ptr + 3] = split_TTL[3];
    free(split_TTL);  
}

// Update query with cache response
void __set_query_response(Query* query, CacheEntry* entry){
    // Update cache id to current request id
    entry->raw_response[ID_PTR] = query->raw_request->data[ID_PTR];
    entry->raw_response[ID_PTR + 1] = query->raw_request->data[ID_PTR + 1];

    // Set query response to cached response
    query->raw_repsonse->data = (char*)malloc(sizeof(query->raw_repsonse->data) *  entry->n); 
    assert(query->raw_repsonse->data);   
    memcpy(query->raw_repsonse->data, entry->raw_response, entry->n);
    
    // Set RDDATA
    query->answer->RDDATA = (char*)malloc(sizeof(query->answer->RDDATA) * INET6_ADDRSTRLEN);
    assert(query->answer->RDDATA);
    strcpy(query->answer->RDDATA, entry->IPv6);

    query->raw_repsonse->n = entry->n;
    
    query->header->QR = 1;
    query->answer->A_RR = 1;
}

// Remove cache entry from list
void __remove_cache_entry(ListCache* list, CacheEntry* entry){
    if (list->head == entry){
        list->head = entry->next;
    }

    if (list->foot == entry){
        list->foot = entry->prev;
    }

    if (entry->next != NULL){
        entry->next->prev = entry->prev;
    }

    if (entry->prev != NULL){
        entry->prev->next = entry->next;
    }

    free(entry);
    list->n -= 1;
}

// Return TTL of given response
int __get_TTL(Query* response){
    int TTL_ptr = __get_QNAME_len(response->raw_repsonse->data) + QNAME_PTR + TTL_PTR_OFFSET;
    short upper = concat_bytes(response->raw_repsonse->data[TTL_ptr], response->raw_repsonse->data[TTL_ptr + 1]);
    short lower = concat_bytes(response->raw_repsonse->data[TTL_ptr + 2], response->raw_repsonse->data[TTL_ptr + 3]);

    return concat_shorts(upper, lower);
}

// Free list memory
void free_list(ListCache* list){
    assert(list);
    CacheEntry* prev, *curr = list->head;

    while(curr){
        prev = curr;
        curr = curr->next;
        free(curr->domain);
        free(curr->IPv6);
        free(curr->raw_response);
        free(prev);
    }
    free(list);
}
