#pragma once

#include "gen_vector.h"
#include "map_setup.h"




typedef struct {
    genVec* buckets; // of ELM
    size_t capacity;
    size_t size;

    size_t elm_size;

    custom_hash_fn hash_fn;
    delete_fn elm_del_fn;
    compare_fn cmp_fn;
} __attribute__((aligned(64))) hashset;


hashset* hashset_create(size_t elm_size, custom_hash_fn hash_fn, 
                        delete_fn elm_del, compare_fn cmp_fn);

void hashset_destroy(hashset* set);

u8 hashset_insert(hashset* set, const u8* elm);

u8 hashset_has(const hashset* set, const u8* elm);

u8 hashset_remove(hashset* set, const u8* elm);

void hashset_print(const hashset* set, genVec_print_fn elm_print);


