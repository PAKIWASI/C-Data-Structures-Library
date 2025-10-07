#pragma once

#include "gen_vector.h"
#include "map_setup.h"



typedef struct {
    u8* elm;
    STATE state;
} __attribute__((aligned(16))) ELM;


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

int hashset_insert(hashset* set, const u8* elm);

int hashset_has(const hashset* set, const u8* elm);

int hashset_remove(hashset* set, const u8* elm);

void hashset_print(const hashset* set, genVec_print_fn elm_print);

size_t hashset_size(const hashset* set) { return set->size; }

