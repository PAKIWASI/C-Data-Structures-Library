#ifndef HASHSET_H
#define HASHSET_H

#include "map_setup.h"


typedef struct {
    u8*             buckets;
    u32             size;
    u32             capacity;
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
 * @param move_fn - Move function for elms (or NULL for default move)
 * @param del_fn - Cleanup function for elms (or NULL if elms don't own resources)
 */
hashset* hashset_create(u16 elm_size, custom_hash_fn hash_fn, compare_fn cmp_fn, 
                         copy_fn copy_fn, move_fn move_fn, delete_fn del_fn);

/**
 * Destroy hashset and clean up all resources
 */
void hashset_destroy(hashset* set);

/**
 * Insert new element in hashset (if not already present) with COPY semantics
 * 
 * @return 1 if element existed (do nothing), 0 if new element inserted
 */
b8 hashset_insert(hashset* set, const u8* elm);

/**
 * Insert new element in hashset (if not already present) with MOVE semantics
 * 
 * @return 1 if element existed (do nothing), 0 if new element inserted
 */
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
 * Print all elements using print_fn
 */
void hashset_print(const hashset* set, map_print_fn print_fn);

/**
 * Clear all elements from set but keep capacity
 */
void hashset_clear(hashset* set);

/**
 * Reset set to initial capacity and remove all elements
 */
void hashset_reset(hashset* set);

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

#endif // HASHSET_H
