#pragma once

#include "gen_vector.h"
#include <stddef.h>



typedef enum {
    EMPTY = 0,
    FILLED = 1,
    TOMBSTONE = 2
} STATE;


typedef void (*kv_delete_fn)(void* key); //optional (for string*, arrays, structs as keys, vals)
typedef size_t (*custom_hash_fn)(const void* key, size_t size);     // optional (for a specific usecase)
typedef int (*key_compare_fn)(const void* a, const void* b, size_t size);   // optional: for compare operations
typedef void (*val_modify_fn)(void* val, const void* delta);                // imp for hashmap_modify


typedef struct {
    void* key;   // KV holds pointers to key, val data
    void* val;
    STATE state;
} __attribute__((aligned(32))) KV;


typedef struct {
    genVec* buckets;    // of KV 
    size_t capacity;     // total number of buckets
    size_t size;         // number of elements

    size_t key_size;    // size of each key
    size_t val_size;    // size of each val

    custom_hash_fn hash_fn;
    kv_delete_fn key_del_fn;
    kv_delete_fn val_del_fn;
    key_compare_fn compare_fn;
} __attribute__((aligned(128))) hashmap;



hashmap* hashmap_create(size_t key_size, size_t val_size, custom_hash_fn hash_fn,
                        kv_delete_fn key_del, kv_delete_fn val_del, key_compare_fn cmp);

void hashmap_destroy(hashmap* map);

int hashmap_put(hashmap* map, const void* key, const void* val);

int hashmap_get(const hashmap* map, const void* key, void* val);

int hashmap_del(hashmap* map, const void* key);

int hashmap_modify(hashmap* map, const void* key, val_modify_fn modify_fn, void* user_data);

int hashmap_has(const hashmap* map, const void* key);

void hashmap_print(hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print);



