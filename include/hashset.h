#pragma once

#include "gen_vector.h"
#include "map_setup.h"



typedef struct {
    genVec*         buckets; // of ELM
    u32             capacity;
    u32             size;

    u16             elm_size;

    custom_hash_fn  hash_fn;
    delete_fn       elm_del_fn;
    compare_fn      cmp_fn;

} hashset;



hashset*  hashset_create(u16 elm_size, custom_hash_fn hash_fn, 
                      delete_fn elm_del, compare_fn cmp_fn);

void      hashset_destroy(hashset* set);

u8        hashset_insert(hashset* set, const u8* elm);

u8        hashset_has(const hashset* set, const u8* elm);

u8        hashset_remove(hashset* set, const u8* elm);

void      hashset_print(const hashset* set, genVec_print_fn elm_print);


