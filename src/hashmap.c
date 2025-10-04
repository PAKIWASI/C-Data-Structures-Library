#include "hashmap.h"
#include "gen_vector.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LOAD_FACTOR_GROW 0.70
#define LOAD_FACTOR_SHRINK 0.20  
#define HASHMAP_INIT_CAPACITY 17  //prime no (index = hash % capacity)


/*
====================DEFAULT FUNCTIONS====================
*/
// 32-bit FNV-1a (default hash)
static size_t fnv1a_hash(const void* data, size_t size) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t hash = 2166136261U;  // FNV offset basis

    for (size_t i = 0; i < size; i++) {
        hash ^= bytes[i];   // XOR with current byte
        hash *= 16777619U;  // Multiply by FNV prime
    }

    return hash;
}

// MurmurHash3 - much faster and better distribution
static size_t murmurhash3(const void* key, size_t size) 
{
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t r1 = 15;
    const uint32_t r2 = 13;
    const uint32_t m = 5;
    const uint32_t n = 0xe6546b64;
    
    const uint8_t* data = (const uint8_t*)key;
    const int nblocks = size / 4;
    uint32_t h1 = 0x9747b28c; // seed
    
    // Body
    const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);
    for(int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i];
        
        k1 *= c1;
        k1 = (k1 << r1) | (k1 >> (32 - r1));
        k1 *= c2;
        
        h1 ^= k1;
        h1 = (h1 << r2) | (h1 >> (32 - r2));
        h1 = h1 * m + n;
    }
    
    // Tail
    const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);
    uint32_t k1 = 0;
    
    switch(size & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2; h1 ^= k1;
    }
    
    // Finalization
    h1 ^= size;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    
    return h1;
}

// default delete function (works for basic data types)
static void default_delete(void* keyORval) {
    if (keyORval) {
        free(keyORval);
    }
}

// Default compare function
static int default_compare(const void* a, const void* b, size_t size) 
{
    return memcmp(a, b, size);
}


/*
====================KV HANDLERS====================
*/

static void kv_destroy(hashmap* map, const KV* kv)
{
    if (!kv) { return; }

    if (kv->key) {
        map->key_del_fn(kv->key); 
    }
    if (kv->val) {
        map->val_del_fn(kv->val);
    }

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
// we use warp around, so when we hit end while probing, we go to the start of arr
static size_t find_slot(const hashmap* map, const void* key,
                        int* found, int* tombstone)
{
    size_t index = map->hash_fn(key, map->key_size) % map->capacity;

    *found = 0;
    *tombstone = -1;

    KV kv;
    for (size_t x = 0; x < map->capacity; x++) {
        size_t i = (index + x) % map->capacity;   // warp around
        genVec_get(map->buckets, i, &kv);

        switch (kv.state) {
            case EMPTY: // bucket empty, found 0 and return bucket as insertion location
                return i;
            case FILLED: // bucket filled, if key match, found 1 and return i
                if (map->compare_fn(kv.key, key, map->key_size) == 0) 
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
    
    return (*tombstone != -1) ? (size_t)*tombstone : 0;
}

static void hashmap_resize(hashmap* map, size_t new_capacity) 
{
    if (!map) {
        printf("map resize: map is null\n");
        return;
    }
    if (new_capacity <= HASHMAP_INIT_CAPACITY) {
        new_capacity = HASHMAP_INIT_CAPACITY;
    }

    genVec* old_vec = map->buckets;

    KV kv = {
        .key = NULL,
        .val = NULL,
        .state = EMPTY
    };
    map->buckets = genVec_init_val(new_capacity, &kv, map->buckets->data_size, NULL);
    if (!map->buckets) {
        printf("map resize: new vec init failed\n");
        map->buckets = old_vec;
        return;
    }

    map->capacity = new_capacity;
    map->size = 0;          // recounted when we rehash

    KV old_kv;
    for (size_t i = 0; i < old_vec->capacity; i++) {
        genVec_get(old_vec, i, &old_kv);
        
        if (old_kv.state == FILLED) {
            int found = 0;
            int tombstone = -1;
            size_t slot = find_slot(map, old_kv.key, &found, &tombstone);

            // new table can't have tombstones, and load factor will be like 35%
            // so no need for error checking
            KV new_kv = {
                .key = old_kv.key,
                .val = old_kv.val,
                .state = FILLED
            };

            genVec_replace(map->buckets, slot, &new_kv);
            map->size++;
        }
    }

    // cleanup old kvs but dont free key/vals
    genVec_destroy(old_vec);
}

static void hashmap_maybe_resize(hashmap* map) {
    if (!map) { return; }
    
    double load_factor = (double)map->size / (double)map->capacity;
    
    if (load_factor > LOAD_FACTOR_GROW) {
        hashmap_resize(map, map->capacity * 2);
    }
    // shrink when too empty
    else if (load_factor < LOAD_FACTOR_SHRINK && map->capacity > HASHMAP_INIT_CAPACITY) 
    {
        hashmap_resize(map, map->capacity / 2);
    }
}

/*
====================PUBLIC FUNCTIONS====================
*/

hashmap* hashmap_create(size_t key_size, size_t val_size, custom_hash_fn hash_fn,
                        kv_delete_fn key_del, kv_delete_fn val_del, key_compare_fn cmp)
{
    if (key_size == 0 || val_size == 0) {
        printf("map create: size cant be 0\n");
        return NULL;
    }

    hashmap* map = malloc(sizeof(hashmap));
    if (!map) {
        printf("map create: map malloc failed\n");
        return NULL;
    }

    KV kv = {     // empty kv that points to null
        .key = NULL,
        .val = NULL,
        .state = EMPTY
    };

    // we dont give custom delete fn for kv as kv is stored directly and not a pointer
    map->buckets = genVec_init_val(HASHMAP_INIT_CAPACITY, &kv, sizeof(KV), NULL);
    if (!map->buckets) {
        printf("map create: buckets init failed\n");
        free(map);
        return NULL;
    }
    
    map->capacity = HASHMAP_INIT_CAPACITY;
    map->size = 0;
    map->key_size = key_size;
    map->val_size = val_size;

    map->hash_fn = hash_fn ? hash_fn : fnv1a_hash;
    map->key_del_fn = key_del ? key_del : default_delete; 
    map->val_del_fn = val_del ? val_del : default_delete;
    map->compare_fn = cmp ? cmp : default_compare;

    return map;
}

void hashmap_destroy(hashmap* map)
{
    if (!map) { return; }

    KV kv;
    if (map->buckets) {
        for (size_t i = 0; i < map->capacity; i++) {
            genVec_get(map->buckets, i, &kv);   
            kv_destroy(map, &kv);
        }
        genVec_destroy(map->buckets);
    }

    free(map);
}


int hashmap_put(hashmap* map, const void* key, const void* val)
{
    if (!map || !key || !val) {
        printf("map put: map/key/val is null\n");
        return -1;
    }
    
    hashmap_maybe_resize(map);
    
    int found = 0;
    int tombstone = -1;
    size_t slot = find_slot(map, key, &found, &tombstone);
    
    if (found) {
        // Key exists - update value
        KV kv;
        genVec_get(map->buckets, slot, &kv);
        
        if (kv.val) {
            map->val_del_fn(kv.val);
        }
        
        kv.val = malloc(map->val_size);
        if (!kv.val) {
            printf("map put: val malloc failed\n");
            return -1;
        }
        memcpy(kv.val, val, map->val_size);
        
        genVec_replace(map->buckets, slot, &kv);
        
        return 0;
    } 
    else {
        // New key - insert
        KV kv = {
            .key = malloc(map->key_size),
            .val = malloc(map->val_size),
            .state = FILLED
        };
        
        if (!kv.key || !kv.val) {
            printf("map put: key/val malloc failed\n");
            if (kv.key) { free(kv.key); }
            if (kv.val) { free(kv.val); }
            return -1;
        }
        
        memcpy(kv.key, key, map->key_size);
        memcpy(kv.val, val, map->val_size);
        
        genVec_replace(map->buckets, slot, &kv);
        map->size++;  
        
        return 0;
    }
}

int hashmap_get(const hashmap* map, const void* key, void* val)
{
    if (!map || !key || !val) {
        printf("map get: map/key is null\n");
        return -1;
    }
    
    int found = 0;
    int tombstone = -1;
    size_t slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        KV kv;
        genVec_get(map->buckets, slot, &kv);
        memcpy(val, kv.val, map->val_size);

        return 0;
    }
    else {
        printf("map get: not found\n");
        return -1;
    }
}


int hashmap_del(hashmap* map, const void* key)
{
    if (map->size == 0) { return -1; }
    if (!map || !key) {
        printf("map del: map/key is null\n");
        return -1;
    }

    int found = 0;
    int tombstone = -1;
    size_t slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        KV kv;
        genVec_get(map->buckets, slot, &kv);
        kv_destroy(map, &kv);

        KV newkv = {
            .key = NULL,
            .val = NULL,
            .state = TOMBSTONE
        };
        genVec_replace(map->buckets, slot, &newkv);
        map->size--;

        hashmap_maybe_resize(map);
        return 0;
    }
    else {
        printf("map del: not found\n");
        return -1;
    }
}

void hashmap_print(hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print)
{
    if (!map || !key_print || !val_print) {
        printf("map print: map is null\n");
        return;
    }

    KV kv;
    printf("\t=========\n");
    for (size_t i = 0; i < map->capacity; i++) {
        genVec_get(map->buckets, i, &kv);
        if (kv.state == FILLED) {
            printf("\t");
            key_print(kv.key);
            printf(" ==> ");
            val_print(kv.val);
            printf("\n");
        }
    }
    printf("\t=========\n");
}


