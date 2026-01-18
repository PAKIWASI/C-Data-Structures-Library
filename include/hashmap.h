#pragma once

#include "gen_vector.h"
#include "map_setup.h"


// Function type definitions


typedef struct {
    genVec*         buckets;        // of KV 
    u32             capacity;       // total number of buckets
    u32             size;           // number of elements

    u16             key_size;       // size of each key
    u16             val_size;       // size of each val

    custom_hash_fn  hash_fn;
    compare_fn      cmp_fn;
    
    // Key semantics
    copy_fn         key_copy_fn;    // Deep copy for keys (or NULL for memcpy)
    move_fn         key_move_fn;    // Move semantics for keys (or NULL)
    delete_fn       key_del_fn;     // Cleanup for key resources (or NULL)
    
    // Value semantics
    copy_fn         val_copy_fn;    // Deep copy for values (or NULL for memcpy)
    move_fn         val_move_fn;    // Move semantics for values (or NULL)
    delete_fn       val_del_fn;     // Cleanup for value resources (or NULL)

} hashmap;


/**
 * Create a new hashmap
 * 
 * @param key_size - Size in bytes of each key
 * @param val_size - Size in bytes of each value
 * @param hash_fn - Custom hash function (or NULL for default FNV-1a)
 * @param cmp_fn - Custom comparison function (or NULL for memcmp)
 * @param key_copy - Deep copy function for keys (or NULL for memcpy)
 * @param val_copy - Deep copy function for values (or NULL for memcpy)
 * @param key_move - Move function for keys (or NULL for default move)
 * @param val_move - Move function for values (or NULL for default move)
 * @param key_del - Cleanup function for keys (or NULL if keys don't own resources)
 * @param val_del - Cleanup function for values (or NULL if values don't own resources)
 */
hashmap* hashmap_create(u16 key_size, u16 val_size, custom_hash_fn hash_fn,
                        compare_fn cmp_fn, copy_fn key_copy, copy_fn val_copy,
                        move_fn key_move, move_fn val_move,
                        delete_fn key_del, delete_fn val_del);

/**
 * Destroy hashmap and clean up all resources
 */
void hashmap_destroy(hashmap* map);

/**
 * Insert or update key-value pair (copy semantics)
 * 
 * @return 1 if key existed (updated), 0 if new key inserted
 */
b8 hashmap_put(hashmap* map, const u8* key, const u8* val);

// TODO: unify this function and pass bools to move key/val or not

b8 hashmap_put_unified(hashmap* map, u8* key, b8 key_move,
                        u8* val, b8 val_move);

/**
 * Insert or update key-value pair (move semantics)
 * Transfers ownership of key and value, nulls source pointers
 * 
 * @param key - Pointer to key pointer (will be nulled)
 * @param val - Pointer to value pointer (will be nulled)
 * @return 1 if key existed (updated), 0 if new key inserted
 */
b8 hashmap_put_move(hashmap* map, u8** key, u8** val);

/**
 * Get value for key (copy semantics)
 * 
 * @param val - Output buffer for value
 * @return 1 if found, 0 if not found
 */
b8 hashmap_get(const hashmap* map, const u8* key, u8* val);

/**
 * Get pointer to value (no copy)
 * 
 * @return Pointer to value or NULL if not found
 * @warning Pointer invalidated by put/del operations
 */
u8* hashmap_get_ptr(hashmap* map, const u8* key);

/**
 * Delete key-value pair
 * 
 * @return 1 if found and deleted, 0 if not found
 */
b8 hashmap_del(hashmap* map, const u8* key);

/**
 * Delete key-value pair and move val to out ptr
 * 
 * @param val - Output buffer for value
 * @return 1 if found and deleted, 0 if not found
 */
b8 hashmap_del_move(hashmap* map, const u8* key, u8* out);

/**
 * Check if key exists
 * 
 * @return 1 if key exists, 0 otherwise
 */
b8 hashmap_has(const hashmap* map, const u8* key);

/**
 * Print all key-value pairs
 */
void hashmap_print(const hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print);


// Inline utility functions
static inline u32 hashmap_size(const hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    return map->size;
}

static inline u32 hashmap_capacity(const hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    return map->capacity;
}

static inline b8 hashmap_empty(const hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    return map->size == 0;
}


