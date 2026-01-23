#pragma once

#include "gen_vector.h"
#include "map_setup.h"


typedef struct {
    genVec*         buckets;        // TODO: replace with raw array? (only using replace, get_ptr and init_val. memory managed by hashmap, not vec)
    u32             capacity;
    u32             size;
    u16             key_size;
    u16             val_size;
    custom_hash_fn  hash_fn;
    compare_fn      cmp_fn;
    copy_fn         key_copy_fn;
    move_fn         key_move_fn;
    delete_fn       key_del_fn;
    copy_fn         val_copy_fn;
    move_fn         val_move_fn;
    delete_fn       val_del_fn;
} hashmap;

/**
 * Create a new hashmap
 */
hashmap* hashmap_create(u16 key_size, u16 val_size, custom_hash_fn hash_fn,
                        compare_fn cmp_fn, copy_fn key_copy, copy_fn val_copy,
                        move_fn key_move, move_fn val_move,
                        delete_fn key_del, delete_fn val_del);

void hashmap_destroy(hashmap* map);

/**
 * Insert or update key-value pair (COPY semantics)
 * Both key and val are passed as const u8*
 * 
 * @return 1 if key existed (updated), 0 if new key inserted
 */
b8 hashmap_put(hashmap* map, const u8* key, const u8* val);

/**
 * Insert or update key-value pair (MOVE semantics)
 * Both key and val are passed as u8** and will be nulled
 * 
 * @return 1 if key existed (updated), 0 if new key inserted
 */
b8 hashmap_put_move(hashmap* map, u8** key, u8** val);

/**
 * Insert with mixed semantics
 */
b8 hashmap_put_val_move(hashmap* map, const u8* key, u8** val);
b8 hashmap_put_key_move(hashmap* map, u8** key, const u8* val);

/**
 * Get value for key (copy semantics)
 */
b8 hashmap_get(const hashmap* map, const u8* key, u8* val);

/**
 * Get pointer to value (no copy)
 * WARNING: Pointer invalidated by put/del operations
 */
u8* hashmap_get_ptr(hashmap* map, const u8* key);

/**
 * Delete key-value pair
 * If out is provided, value is copied to it before deletion
 */
b8 hashmap_del(hashmap* map, const u8* key, u8* out);


// TODO: 
void hashmap_clear(hashmap* map);  // Remove all, keep capacity
void hashmap_reset(hashmap* map);  // Remove all, reset to initial capacity
// Update value in-place if key exists, return false if key doesn't exist
b8 hashmap_update(hashmap* map, const u8* key, const u8* val);
genVec* hashmap_keys(const hashmap* map);
genVec* hashmap_values(const hashmap* map);

/**
 * Check if key exists
 */
b8 hashmap_has(const hashmap* map, const u8* key);

/**
 * Print all key-value pairs
 */
void hashmap_print(const hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print);

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


