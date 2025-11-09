#pragma once

#include "gen_vector.h"
#include "map_setup.h"




typedef struct {
    genVec* buckets;    // of KV 
    size_t capacity;     // total number of buckets
    size_t size;         // number of elements

    size_t key_size;    // size of each key
    size_t val_size;    // size of each val

    custom_hash_fn hash_fn;
    delete_fn key_del_fn;
    delete_fn val_del_fn;
    compare_fn compare_fn;
} __attribute__((aligned(128))) hashmap;


hashmap* hashmap_create(size_t key_size, size_t val_size, custom_hash_fn hash_fn,
                        delete_fn key_del, delete_fn val_del, compare_fn cmp);

void hashmap_destroy(hashmap* map);

u8 hashmap_put(hashmap* map, const u8* key, const u8* val);

u8 hashmap_get(const hashmap* map, const u8* key, u8* val);

u8 hashmap_del(hashmap* map, const u8* key);

u8 hashmap_modify(hashmap* map, const u8* key, val_modify_fn modify_fn, u8* user_data);

u8 hashmap_has(const hashmap* map, const u8* key);

void hashmap_print(const hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print);

