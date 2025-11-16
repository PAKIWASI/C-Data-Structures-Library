#pragma once

#include "gen_vector.h"
#include "map_setup.h"


typedef struct {
    genVec* buckets;    // of KV 
    u32 capacity;     // total number of buckets
    u32 size;         // number of elements

    u32 key_size;    // size of each key
    u32 val_size;    // size of each val

    custom_hash_fn hash_fn;
    delete_fn key_del_fn;
    delete_fn val_del_fn;
    compare_fn compare_fn;
} __attribute__((aligned(64))) hashmap;


hashmap* hashmap_create(u32 key_size, u32 val_size, custom_hash_fn hash_fn,
                        delete_fn key_del, delete_fn val_del, compare_fn cmp);

void hashmap_destroy(hashmap* map);

u8 hashmap_put(hashmap* map, const u8* key, const u8* val);

u8 hashmap_get(const hashmap* map, const u8* key, u8* val);

u8* hashmap_get_ptr(hashmap* map, const u8* key);

u8 hashmap_del(hashmap* map, const u8* key);

u8 hashmap_has(const hashmap* map, const u8* key);

void hashmap_print(const hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print);


