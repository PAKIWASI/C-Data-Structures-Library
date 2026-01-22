#include "hashmap.h"
#include "common.h"

typedef struct {
    u8* key;
    u8* val;
    STATE state;
} KV;

/*
====================KV HANDLERS====================
*/

static void kv_destroy(hashmap* map, const KV* kv)
{
    CHECK_FATAL(!kv, "kv is null");

    if (kv->key) {
        if (map->key_del_fn) {
            map->key_del_fn(kv->key); 
        }
        free(kv->key);
    }

    if (kv->val) {
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        // free(kv->val);
        map->buckets->allocator->free_fn(kv->val);
    }
}

/*
====================PRIVATE FUNCTIONS====================
*/

// CRITICAL: find_slot ALWAYS takes const u8* key (not u8**)
// This ensures consistent hashing regardless of caller's pointer type
static u32 find_slot(const hashmap* map, const u8* key,
                        b8* found, int* tombstone)
{
    u32 index = map->hash_fn(key, map->key_size) % map->capacity;

    *found = 0;
    *tombstone = -1;

    for (u32 x = 0; x < map->capacity; x++) {
        u32 i = (index + x) % map->capacity;
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, i);

        switch (kv->state) {
            case EMPTY:
                return i;
            case FILLED:
                if (map->cmp_fn(kv->key, key, map->key_size) == 0) 
                {
                    *found = 1;
                    return i;
                }
                break;
            case TOMBSTONE:
                if (*tombstone == -1) {
                    *tombstone = (int)i;
                }
                break;
        }
    }
    
    return (*tombstone != -1) ? (u32)*tombstone : 0;
}

static void hashmap_resize(hashmap* map, u32 new_capacity) 
{
    if (new_capacity <= HASHMAP_INIT_CAPACITY) {
        new_capacity = HASHMAP_INIT_CAPACITY;
    }

    genVec* old_vec = map->buckets;

    KV kv = { .key = NULL, .val = NULL, .state = EMPTY };
    map->buckets = genVec_init_val(new_capacity, cast(kv), sizeof(KV), NULL, NULL, NULL);

    map->capacity = new_capacity;
    map->size = 0;

    for (u32 i = 0; i < old_vec->capacity; i++) {
        const KV* old_kv = (const KV*)genVec_get_ptr(old_vec, i);
        
        if (old_kv->state == FILLED) {
            b8 found = 0;
            int tombstone = -1;
            u32 slot = find_slot(map, old_kv->key, &found, &tombstone);

            KV new_kv = {
                .key = old_kv->key,
                .val = old_kv->val,
                .state = FILLED
            };

            genVec_replace(map->buckets, slot, (u8*)&new_kv);
            map->size++;
        }
    }

    genVec_destroy(old_vec);
}

static void hashmap_maybe_resize(hashmap* map) 
{
    CHECK_FATAL(!map, "map is null");
    
    double load_factor = (double)map->size / (double)map->capacity;
    
    if (load_factor > LOAD_FACTOR_GROW) {
        u32 new_cap = next_prime(map->capacity);
        hashmap_resize(map, new_cap);
    }
    else if (load_factor < LOAD_FACTOR_SHRINK && map->capacity > HASHMAP_INIT_CAPACITY) 
    {
        u32 new_cap = prev_prime(map->capacity);
        if (new_cap >= HASHMAP_INIT_CAPACITY) {
            hashmap_resize(map, new_cap);
        }
    }
}

/*
====================PUBLIC FUNCTIONS====================
*/

hashmap* hashmap_create(u16 key_size, u16 val_size, custom_hash_fn hash_fn,
                        compare_fn cmp_fn, copy_fn key_copy, copy_fn val_copy,
                        move_fn key_move, move_fn val_move,
                        delete_fn key_del, delete_fn val_del)
{
    CHECK_FATAL(key_size == 0, "key size can't be zero");
    CHECK_FATAL(val_size == 0, "val size can't be zero");

    hashmap* map = malloc(sizeof(hashmap));
    CHECK_FATAL(!map, "map malloc failed");

    KV kv = { .key = NULL, .val = NULL, .state = EMPTY };
    map->buckets = genVec_init_val(HASHMAP_INIT_CAPACITY, cast(kv), sizeof(KV), NULL, NULL, NULL);
    
    map->capacity = HASHMAP_INIT_CAPACITY;
    map->size = 0;
    map->key_size = key_size;
    map->val_size = val_size;

    map->hash_fn = hash_fn ? hash_fn : fnv1a_hash;
    map->cmp_fn = cmp_fn ? cmp_fn : default_compare;
    
    map->key_copy_fn = key_copy;
    map->key_move_fn = key_move;
    map->key_del_fn = key_del;
    
    map->val_copy_fn = val_copy;
    map->val_move_fn = val_move;
    map->val_del_fn = val_del;

    return map;
}

void hashmap_destroy(hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!map->buckets, "map bucket is null");

    for (u32 i = 0; i < map->capacity; i++) {
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, i);
        if (kv->state == FILLED) {
            kv_destroy(map, kv);
        }
    }
    genVec_destroy(map->buckets);
    free(map);
}

// COPY semantics - key and val are const u8*
b8 hashmap_put(hashmap* map, const u8* key, const u8* val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!val, "val is null");

    hashmap_maybe_resize(map);
    
    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);
    
    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);
        
        // Free old value's resources
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        
        // Update value
        if (map->val_copy_fn) {
            map->val_copy_fn(kv->val, val);
        } else {
            memcpy(kv->val, val, map->val_size);
        }
        
        return 1; // found - updated
    } 
    
    // New key - insert
    KV kv = {
        .key = map->buckets->allocator->alloc_fn(map->key_size),
        .val = map->buckets->allocator->alloc_fn(map->val_size),
        .state = FILLED
    };
    
    CHECK_FATAL(!kv.key, "key malloc failed");
    CHECK_FATAL(!kv.val, "val malloc failed");
    
    // Copy key
    if (map->key_copy_fn) {
        map->key_copy_fn(kv.key, key);
    } else {
        memcpy(kv.key, key, map->key_size);
    }
    
    // Copy value
    if (map->val_copy_fn) {
        map->val_copy_fn(kv.val, val);
    } else {
        memcpy(kv.val, val, map->val_size);
    }
    
    genVec_replace(map->buckets, slot, (u8*)&kv);
    map->size++;  
    
    return 0;
}

// MOVE semantics - key and val are u8**
b8 hashmap_put_move(hashmap* map, u8** key, u8** val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!*key, "*key is null");
    CHECK_FATAL(!val, "val is null");
    CHECK_FATAL(!*val, "*val is null");
    
    hashmap_maybe_resize(map);
    
    b8 found = 0;
    int tombstone = -1;
    // IMPORTANT: Dereference *key to pass u8* to find_slot
    u32 slot = find_slot(map, *key, &found, &tombstone);
    
    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);
        
        // Free old value's resources
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        
        // Move value
        if (map->val_move_fn) {
            map->val_move_fn(kv->val, val);
        } else {
            memcpy(kv->val, *val, map->val_size);
            *val = NULL;
        }
        
        // Key already exists, clean up the passed key
        if (map->key_del_fn) {
            map->key_del_fn(*key);
        }
        free(*key);
        *key = NULL;
        
        return 1;
    }
    
    // New key - insert with move semantics
    KV kv = {
        .key = map->buckets->allocator->alloc_fn(map->key_size),
        .val = map->buckets->allocator->alloc_fn(map->val_size),
        .state = FILLED
    };
    
    CHECK_FATAL(!kv.key, "key malloc failed");
    CHECK_FATAL(!kv.val, "val malloc failed");
    
    // Move key
    if (map->key_move_fn) {
        map->key_move_fn(kv.key, key);
    } else {
        memcpy(kv.key, *key, map->key_size);
        *key = NULL;
    }
    
    // Move value
    if (map->val_move_fn) {
        map->val_move_fn(kv.val, val);
    } else {
        memcpy(kv.val, *val, map->val_size);
        *val = NULL;
    }
    
    genVec_replace(map->buckets, slot, (u8*)&kv);
    map->size++;
    
    return 0;
}

// Mixed: key copy, val move
b8 hashmap_put_val_move(hashmap* map, const u8* key, u8** val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!val, "val is null");
    CHECK_FATAL(!*val, "*val is null");
    
    hashmap_maybe_resize(map);
    
    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);
    
    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);
        
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        
        if (map->val_move_fn) {
            map->val_move_fn(kv->val, val);
        } else {
            memcpy(kv->val, *val, map->val_size);
            *val = NULL;
        }
        
        return 1;
    }
    
    KV kv = {
        .key = map->buckets->allocator->alloc_fn(map->key_size),
        .val = map->buckets->allocator->alloc_fn(map->val_size),
        .state = FILLED
    };
    
    CHECK_FATAL(!kv.key, "key malloc failed");
    CHECK_FATAL(!kv.val, "val malloc failed");
    
    if (map->key_copy_fn) {
        map->key_copy_fn(kv.key, key);
    } else {
        memcpy(kv.key, key, map->key_size);
    }
    
    if (map->val_move_fn) {
        map->val_move_fn(kv.val, val);
    } else {
        memcpy(kv.val, *val, map->val_size);
        *val = NULL;
    }
    
    genVec_replace(map->buckets, slot, (u8*)&kv);
    map->size++;
    
    return 0;
}

// Mixed: key move, val copy
b8 hashmap_put_key_move(hashmap* map, u8** key, const u8* val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!*key, "*key is null");
    CHECK_FATAL(!val, "val is null");
    
    hashmap_maybe_resize(map);
    
    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, *key, &found, &tombstone);
    
    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);
        
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        
        if (map->val_copy_fn) {
            map->val_copy_fn(kv->val, val);
        } else {
            memcpy(kv->val, val, map->val_size);
        }
        
        if (map->key_del_fn) {
            map->key_del_fn(*key);
        }
        free(*key);
        *key = NULL;
        
        return 1;
    }
    
    KV kv = {
        .key = map->buckets->allocator->alloc_fn(map->key_size),
        .val = map->buckets->allocator->alloc_fn(map->val_size),
        .state = FILLED
    };
    
    CHECK_FATAL(!kv.key, "key malloc failed");
    CHECK_FATAL(!kv.val, "val malloc failed");
    
    if (map->key_move_fn) {
        map->key_move_fn(kv.key, key);
    } else {
        memcpy(kv.key, *key, map->key_size);
        *key = NULL;
    }
    
    if (map->val_copy_fn) {
        map->val_copy_fn(kv.val, val);
    } else {
        memcpy(kv.val, val, map->val_size);
    }
    
    genVec_replace(map->buckets, slot, (u8*)&kv);
    map->size++;
    
    return 0;
}

b8 hashmap_get(const hashmap* map, const u8* key, u8* val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!val, "val is null");
    
    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, slot);
        
        if (map->val_copy_fn) {
            map->val_copy_fn(val, kv->val);
        } else {
            memcpy(val, kv->val, map->val_size);
        }

        return 1;
    }

    return 0;
}

u8* hashmap_get_ptr(hashmap* map, const u8* key)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        return ((const KV*)genVec_get_ptr(map->buckets, slot))->val;
    } 

    return NULL;
}

b8 hashmap_del(hashmap* map, const u8* key, u8* out)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");

    if (map->size == 0) { return 0; }

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);

        if (out) {
            if (map->val_copy_fn) {
                map->val_copy_fn(out, kv->val);
            } else {
                memcpy(out, kv->val, map->val_size);
            }
        }
        
        kv_destroy(map, kv);

        KV newkv = { .key = NULL, .val = NULL, .state = TOMBSTONE };
        genVec_replace(map->buckets, slot, (u8*)&newkv);
        map->size--;

        hashmap_maybe_resize(map);

        return 1;
    }

    return 0;
}

b8 hashmap_has(const hashmap* map, const u8* key)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    
    b8 found = 0;
    int tombstone = -1;
    find_slot(map, key, &found, &tombstone);
    
    return found;
}

void hashmap_print(const hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key_print, "key_print is null");
    CHECK_FATAL(!val_print, "val_print is null");

    printf("\t=========\n");
    printf("\tSize: %u / Capacity: %u\n", map->size, map->capacity);
    printf("\t=========\n");

    for (u32 i = 0; i < map->capacity; i++) {
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, i);
        if (kv->state == FILLED) {
            printf("\t");
            key_print(kv->key);
            printf(" => ");
            val_print(kv->val);
            printf("\n");
        }
    }

    printf("\t=========\n");
}



