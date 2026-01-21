#pragma once

#include "gen_vector.h"
#include "map_setup.h"



typedef struct {
    genVec*         buckets; // of ELM
    u32             capacity;
    u32             size;

    u16             elm_size;

    custom_hash_fn  hash_fn;
    compare_fn      cmp_fn;

    copy_fn         copy_fn;
    move_fn         move_fn;
    delete_fn       del_fn;

} hashset;



/**
 * Create a new hashset
 * 
 * @param elm_size - Size in bytes of each element
 * @param hash_fn - Custom hash function (or NULL for default FNV-1a)
 * @param cmp_fn - Custom comparison function (or NULL for memcmp)
 * @param copy_fn - Deep copy function for elms (or NULL for memcpy)
 * @param move_fn- Move function for elms (or NULL for default move)
 * @param del_fn - Cleanup function for keys (or NULL if elms don't own resources)
 */
hashset* hashset_create(u16 elm_size, custom_hash_fn hash_fn, compare_fn cmp_fn, 
                         copy_fn copy_fn, move_fn move_fn, delete_fn del_fn) ;

/**
 * Destroy hashset and clean up all resources
 */
void hashset_destroy(hashset* set);

/**
 * Insert new element in hashset (if not already present) 
 * 
 * @param elm      - Pointer to elm 
 * @param elm_move - 0: elm copy semantics (u8*), 1: elm move semantics (u8**) 
 * @return 1 if key existed (do nothing), 0 if new key inserted
 */
b8 hashset_insert(hashset* set, const u8* elm);
b8 hashset_insert_move(hashset* set, u8** elm);

/**
 * Check if elm is present in hashset
 * 
 * @return 1 if found, 0 if not found
 */
b8 hashset_has(const hashset* set, const u8* elm);

/**
 * Delete elm from hashset
 * 
 * @return 1 if found and deleted, 0 if not found
 */
b8 hashset_remove(hashset* set, const u8* elm);

/**
 * Print all elms using print_fn
 */
void hashset_print(const hashset* set, genVec_print_fn print_fn);



// Inline utility functions
static inline u32 hashset_size(const hashset* set)
{
    CHECK_FATAL(!set, "set is null");
    return set->size;
}

static inline u32 hashset_capacity(const hashset* set)
{
    CHECK_FATAL(!set, "set is null");
    return set->capacity;
}

static inline b8 hashset_empty(const hashset* set)
{
    CHECK_FATAL(!set, "set is null");
    return set->size == 0;
}



