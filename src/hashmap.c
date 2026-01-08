#include "hashmap.h"
#include "common.h"
#include "default_functions.h"


typedef struct {
    u8* key;   // KV holds pointers to key, val data
    u8* val;
    STATE state;
} KV;


/*
====================KV HANDLERS====================
*/

static void kv_destroy(hashmap* map, const KV* kv)
{
    CHECK_FATAL(!kv, "kv is null");
    CHECK_FATAL(!kv->key, "kv key is null");
    CHECK_FATAL(!kv->val, "kv val is null");

    if (map->key_del_fn) {
        map->key_del_fn(kv->key); 
    } else {
        free(kv->key);
    }

    if (map->val_del_fn) {
        map->val_del_fn(kv->val);
    } else {
        free(kv->val);
    }
    // now key/val del funcs have to free key/vals themselves

    // dont free kv as managed by genVec
}


/*
====================PRIVATE FUNCTIONS====================
*/


// Find slot for key using linear probing
// Returns: index where key is found or should be inserted
// Sets found_index to 1 if key exists, 0 otherwise
// Sets tombstone_index to first tombstone encountered (for insertion optimization)
// if linear probing fails(find no empty), we insert at the first tombstone encountered
// we warp around, so when we hit end while probing, we go to the start of arr
static u32 find_slot(const hashmap* map, const u8* key,
                        b8* found, int* tombstone)
{
    u32 index = map->hash_fn(key, map->key_size) % map->capacity;

    *found = 0;
    *tombstone = -1;

    for (u32 x = 0; x < map->capacity; x++) {
        u32 i = (index + x) % map->capacity;   // warp around
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, i);

        switch (kv->state) {
            case EMPTY: // bucket empty, found 0 and return bucket as insertion location
                return i;
            case FILLED: // bucket filled, if key match, found 1 and return i
                if (map->compare_fn(kv->key, key, map->key_size) == 0) 
                {
                    *found = 1;
                    return i;
                } // else continue probing to find empty slot or tombstone
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

    KV kv = {
        .key = NULL,
        .val = NULL,
        .state = EMPTY
    };

    map->buckets = genVec_init_val(new_capacity, (u8*)&kv, map->buckets->data_size, NULL);
    // genVec init val fails and kills program or returns correct vec
    //CHECK_FATAL(!map->buckets, "bucket init failed");

    map->capacity = new_capacity;
    map->size = 0;          // recounted when we rehash

    for (u32 i = 0; i < old_vec->capacity; i++) {
        const KV* old_kv = (const KV*)genVec_get_ptr(old_vec, i);
        
        if (old_kv->state == FILLED) {
            b8 found = 0;
            int tombstone = -1;
            u32 slot = find_slot(map, old_kv->key, &found, &tombstone);

            // new table can't have tombstones, and load factor will be like 35%
            // so no need for error checking
            KV new_kv = {
                .key = old_kv->key,
                .val = old_kv->val,
                .state = FILLED
            };

            genVec_replace(map->buckets, slot, (u8*)&new_kv);
            map->size++;
        }
    }

    // cleanup old kvs but dont free key/vals
    genVec_destroy(old_vec); // this will just destroy the KV containers and not the key,val pointers
}

static void hashmap_maybe_resize(hashmap* map) 
{
    CHECK_FATAL(!map, "map is null");
    
    double load_factor = (double)map->size / (double)map->capacity;
    
    if (load_factor > LOAD_FACTOR_GROW) {
        u32 new_cap = next_prime(map->capacity);
        hashmap_resize(map, new_cap);
    }

    // Shrink when too empty
    else if (load_factor < LOAD_FACTOR_SHRINK && map->capacity > HASHMAP_INIT_CAPACITY) 
    {
        u32 new_cap = prev_prime(map->capacity);
        // Don't shrink below initial capacity
        if (new_cap >= HASHMAP_INIT_CAPACITY) {
            hashmap_resize(map, new_cap);
        }
    }
}

/*
====================PUBLIC FUNCTIONS====================
*/

hashmap* hashmap_create(u16 key_size, u16 val_size, custom_hash_fn hash_fn,
                        delete_fn key_del, delete_fn val_del, compare_fn cmp)
{
    CHECK_FATAL(key_size == 0, "key size can't be zero");
    CHECK_FATAL(val_size == 0, "val size can't be zero");

    hashmap* map = malloc(sizeof(hashmap));
    CHECK_FATAL(!map, "map malloc failed");

    KV kv = {     // empty kv that points to null
        .key = NULL,
        .val = NULL,
        .state = EMPTY
    };

    // we dont give custom delete fn for kv as kv is stored directly and not a pointer
    // resources owned by kv are cleaned by us, not genVec destroy
    // this is done because when resizing, we destroy the old container but dont free the mem kv point to
    map->buckets = genVec_init_val(HASHMAP_INIT_CAPACITY, (u8*)&kv, sizeof(KV), NULL);
    //CHECK_FATAL(!map->buckets, "bucket init failed");
    
    map->capacity = HASHMAP_INIT_CAPACITY;
    map->size = 0;
    map->key_size = key_size;
    map->val_size = val_size;

    map->hash_fn = hash_fn ? hash_fn : fnv1a_hash;
    map->key_del_fn = key_del; 
    map->val_del_fn = val_del;
    map->compare_fn = cmp ? cmp : default_compare;

    return map;
}

void hashmap_destroy(hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!map->buckets, "map bucket is null");

    // destroy each kv pair (if kv own resources, clean them)
    for (u32 i = 0; i < map->capacity; i++) {
        kv_destroy(map, (const KV*)genVec_get_ptr(map->buckets, i));
    }
    // destroy the KV containers them selves (no del fn)
    genVec_destroy(map->buckets);

    free(map);
}


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
        
        // If we have a delete function, the value owns resources
        if (map->val_del_fn) {
            // Free old value's resources
            map->val_del_fn(kv->val);
        }
        
        // Update value in place (memory already allocated)
        memcpy(kv->val, val, map->val_size);
        
        return 1; // found
    } 
    else {
        // New key - insert
        KV kv = {
            .key = malloc(map->key_size),
            .val = malloc(map->val_size),
            .state = FILLED
        };
        
        CHECK_FATAL(!kv.key, "key malloc failed");
        CHECK_FATAL(!kv.val, "val malloc failed");
        
        memcpy(kv.key, key, map->key_size);
        memcpy(kv.val, val, map->val_size);
        
        genVec_replace(map->buckets, slot, (u8*)&kv);
        map->size++;  
        
        return 0;  // not found -- new KV
    }
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
        memcpy(val, kv->val, map->val_size);

        return 1; //found
    }
    else {
        return 0; // not found
    }
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
    else {
        return NULL;
    }
}



b8 hashmap_del(hashmap* map, const u8* key)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");

    if (map->size == 0) { return -1; }

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, slot);
        kv_destroy(map, kv);

        KV newkv = {
            .key = NULL,
            .val = NULL,
            .state = TOMBSTONE
        };
        genVec_replace(map->buckets, slot, (u8*)&newkv);
        map->size--;

        hashmap_maybe_resize(map);

        return 1; // found
    }
    else {
        return 0; // not found
    }
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

    for (u32 i = 0; i < map->capacity; i++) {

        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, i);
        if (kv->state == FILLED) {
            printf("\t");
            key_print(kv->key);
            printf(" ==> ");
            val_print(kv->val);
            printf("\n");
        }
    }

    printf("\t=========\n");
}


