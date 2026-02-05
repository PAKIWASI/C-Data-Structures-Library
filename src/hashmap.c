#include "hashmap.h"
#include "map_setup.h"

#include <stdlib.h>
#include <string.h>


#define GET_KV(data, i) ((KV*)(data) + i)


typedef struct {
    u8* key;
    u8* val;
    STATE state;
} KV;

/*
====================KV HANDLERS====================
*/

static void kv_destroy(delete_fn key_del, delete_fn val_del, const KV* kv)
{
    CHECK_FATAL(!kv, "kv is null");

    if (kv->key) {
        if (key_del) {
            key_del(kv->key); 
        }
        free(kv->key);
    }

    if (kv->val) {
        if (val_del) {
            val_del(kv->val);
        }
        free(kv->val);
    }
}

/*
====================PRIVATE FUNCTIONS====================
*/

static void reset_buckets(u8* buckets, u32 size)
{
    KV kv = { 
        .key = NULL, 
        .val = NULL, 
        .state = EMPTY 
    };

    for (u32 i = 0; i < size; i++) {
        memcpy(GET_KV(buckets, i), &kv, sizeof(KV));
    }
}


static void destroy_data(delete_fn key_del, delete_fn val_del, u8* buckets, u32 cap)
{
    // if KV own memory, free it
    for (u32 i = 0; i < cap; i++) {
        const KV* kv = GET_KV(buckets, i);
        if (kv->state == FILLED) {
            kv_destroy(key_del, val_del, kv); // we dont modify map
        }
    }
    free(buckets);
}


static u32 find_slot(const hashmap* map, const u8* key,
                        b8* found, int* tombstone)
{
    u32 index = map->hash_fn(key, map->key_size) % map->capacity;

    *found = 0;
    *tombstone = -1;

    for (u32 x = 0; x < map->capacity; x++) 
    {
        u32 i = (index + x) % map->capacity;
        const KV* kv = GET_KV(map->buckets, i);

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

    u8* old_vec = map->buckets;
    u32 old_cap = map->capacity;

    map->buckets = malloc(new_capacity * sizeof(KV));
    reset_buckets(map->buckets, new_capacity);

    map->capacity = new_capacity;
    map->size = 0;


    for (u32 i = 0; i < old_cap; i++) 
    {
        const KV* old_kv = GET_KV(old_vec, i);
        
        if (old_kv->state == FILLED) {
            b8 found = 0;
            int tombstone = -1;
            u32 slot = find_slot(map, old_kv->key, &found, &tombstone);

            KV* new_kv = GET_KV(map->buckets, slot);
            new_kv->key = old_kv->key;
            new_kv->val = old_kv->val;
            new_kv->state = FILLED;

            map->size++;
        }
    }

    destroy_data(map->key_del_fn, map->val_del_fn, old_vec, old_cap);
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

    map->buckets = malloc(HASHMAP_INIT_CAPACITY * sizeof(KV));
    CHECK_FATAL(!map->buckets, "map bucket init failed");

    reset_buckets(map->buckets, HASHMAP_INIT_CAPACITY);

    
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

    // if KV own memory, free it
    for (u32 i = 0; i < map->capacity; i++) {
        const KV* kv = GET_KV(map->buckets, i);
        if (kv->state == FILLED) {
            kv_destroy(map->key_del_fn, map->val_del_fn, kv);
        }
    }

    free(map->buckets); // free KV container
    free(map);          // free struct
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
        KV* kv = GET_KV(map->buckets, slot);
        
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

    u8* k = malloc(map->key_size);
    CHECK_FATAL(!k, "key malloc failed");
    u8* v = malloc(map->val_size);
    CHECK_FATAL(!v, "val malloc failed");
    
    
    // Copy key
    if (map->key_copy_fn) {
        map->key_copy_fn(k, key);
    } else {
        memcpy(k, key, map->key_size);
    }
    
    // Copy value
    if (map->val_copy_fn) {
        map->val_copy_fn(v, val);
    } else {
        memcpy(v, val, map->val_size);
    }
    
    KV* old_kv = GET_KV(map->buckets, slot); 
    old_kv->key = k;
    old_kv->val = v;
    old_kv->state = FILLED;

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
        KV* kv = GET_KV(map->buckets, slot);
        
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
    u8* k = malloc(map->key_size);
    CHECK_FATAL(!k, "key malloc failed");
    u8* v = malloc(map->val_size);
    CHECK_FATAL(!v, "val malloc failed");
    
    
    // Move key
    if (map->key_move_fn) {
        map->key_move_fn(k, key);
    } else {
        memcpy(k, *key, map->key_size);
        *key = NULL;
    }
    
    // Move value
    if (map->val_move_fn) {
        map->val_move_fn(v, val);
    } else {
        memcpy(v, *val, map->val_size);
        *val = NULL;
    }

    KV* old_kv = GET_KV(map->buckets, slot);
    old_kv->key = k;
    old_kv->val = v;
    old_kv->state = FILLED;
    
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
        KV* kv = (KV*)GET_KV(map->buckets, slot);
        
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
    
    u8* k = malloc(map->key_size);
    CHECK_FATAL(!k, "key malloc failed");
    u8* v = malloc(map->val_size);
    CHECK_FATAL(!v, "val malloc failed");
    
    
    if (map->key_copy_fn) {
        map->key_copy_fn(k, key);
    } else {
        memcpy(k, key, map->key_size);
    }
    
    if (map->val_move_fn) {
        map->val_move_fn(v, val);
    } else {
        memcpy(v, *val, map->val_size);
        *val = NULL;
    }

    KV* old_kv = GET_KV(map->buckets, slot);
    
    old_kv->key = k;
    old_kv->val = v;
    old_kv->state = FILLED;
    
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
        KV* kv = GET_KV(map->buckets, slot);
        
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
    
    u8* k = malloc(map->key_size);
    CHECK_FATAL(!k, "key malloc failed");
    u8* v = malloc(map->val_size);
    CHECK_FATAL(!v, "val malloc failed");
    
    
    if (map->key_move_fn) {
        map->key_move_fn(k, key);
    } else {
        memcpy(k, *key, map->key_size);
        *key = NULL;
    }
    
    if (map->val_copy_fn) {
        map->val_copy_fn(v, val);
    } else {
        memcpy(v, val, map->val_size);
    }

    KV* old_kv = GET_KV(map->buckets, slot);

    old_kv->key = k;
    old_kv->val = v;
    old_kv->state = FILLED;
    
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
        const KV* kv = GET_KV(map->buckets, slot);
        
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
        return (GET_KV(map->buckets, slot))->val;
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
        KV* kv = GET_KV(map->buckets, slot);

        if (out) {
            if (map->val_copy_fn) {
                map->val_copy_fn(out, kv->val);
            } else {
                memcpy(out, kv->val, map->val_size);
            }
        }
        
        kv_destroy(map->key_del_fn, map->val_del_fn, kv);

        kv->key = NULL;
        kv->val = NULL;
        kv->state = TOMBSTONE;

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

void hashmap_print(const hashmap* map, map_print_fn key_print, map_print_fn val_print)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key_print, "key_print is null");
    CHECK_FATAL(!val_print, "val_print is null");

    printf("\t=========\n");
    printf("\tSize: %u / Capacity: %u\n", map->size, map->capacity);
    printf("\t=========\n");

    for (u32 i = 0; i < map->capacity; i++) {
        const KV* kv = GET_KV(map->buckets, i);
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



