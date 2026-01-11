#include "hashset.h"
#include "default_functions.h"
#include "gen_vector.h"
#include "map_setup.h"




typedef struct {
    u8* elm;
    STATE state;
} ELM;


/*
====================ELM HANDLERS====================
*/

static void elm_destroy(hashset* set, const ELM* elm)
{
    CHECK_FATAL(!elm, "ELM is null");
    CHECK_FATAL(!elm->elm, "elm ptr is null");

    if (set->elm_del_fn) {
        set->elm_del_fn(elm->elm);
    } else {
        free(elm->elm);
    }

    // dont free elm
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
static u32 find_slot(const hashset* set, const u8* key,
                        b8* found, int* tombstone)
{
    u32 index = set->hash_fn(key, set->elm_size) % set->capacity;

    *found = 0;
    *tombstone = -1;

    for (u32 x = 0; x < set->capacity; x++) {
        u32 i = (index + x) % set->capacity;   // warp around
        const ELM* elm = (const ELM*)genVec_get_ptr(set->buckets, i);

        switch (elm->state) {
            case EMPTY: // bucket empty, found 0 and return bucket as insertion location
                return i;
            case FILLED: // bucket filled, if key match, found 1 and return i
                if (set->cmp_fn(elm->elm, key, set->elm_size) == 0) 
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


static void hashset_resize(hashset* set, u32 new_capacity) 
{
    if (new_capacity <= HASHMAP_INIT_CAPACITY) {
        new_capacity = HASHMAP_INIT_CAPACITY;
    }

    genVec* old_vec = set->buckets;

    ELM elm = {
        .elm = NULL,
        .state = EMPTY
    };

    set->buckets = genVec_init_val(new_capacity, (u8*)&elm, set->buckets->data_size, NULL, NULL);
    //CHECK_FATAL(!set->buckets, "set bucket init failed");

    set->capacity = new_capacity;
    set->size = 0;          // recounted when we rehash

    for (u32 i = 0; i < old_vec->capacity; i++) {
        const ELM* old_elm = (const ELM*)genVec_get_ptr(old_vec, i);
        
        if (old_elm->state == FILLED) {
            b8 found = 0;
            int tombstone = -1;
            u32 slot = find_slot(set, old_elm->elm, &found, &tombstone);

            // new table can't have tombstones, and load factor will be like 35%
            // so no need for error checking
            ELM new_elm = {
                .elm = old_elm->elm,
                .state = FILLED
            };

            genVec_replace(set->buckets, slot, (u8*)&new_elm);
            set->size++;
        }
    }

    // cleanup old kvs but dont free key/vals
    genVec_destroy(old_vec);
}


static void hashset_maybe_resize(hashset* set) 
{
    CHECK_FATAL(!set, "set is null");
    
    double load_factor = (double)set->size / (double)set->capacity;
    
    if (load_factor > LOAD_FACTOR_GROW) {
        u32 new_cap = next_prime(set->capacity);
        hashset_resize(set, new_cap);
    }

    // Shrink when too empty
    else if (load_factor < LOAD_FACTOR_SHRINK && set->capacity > HASHMAP_INIT_CAPACITY) 
    {
        u32 new_cap = prev_prime(set->capacity);
        // Don't shrink below initial capacity
        if (new_cap >= HASHMAP_INIT_CAPACITY) {
            hashset_resize(set, new_cap);
        }
    }
}

/*
====================PUBLIC FUNCTIONS====================
*/


hashset* hashset_create(u16 elm_size, custom_hash_fn hash_fn, 
                        delete_fn elm_del, compare_fn cmp_fn)
{
    CHECK_FATAL(elm_size == 0, "elm size can't be 0");

    hashset* set = malloc(sizeof(hashset));
    CHECK_FATAL(!set, "set malloc failed");

    ELM elm = {
        .elm = NULL,
        .state = EMPTY
    };

    set->buckets = genVec_init_val(HASHMAP_INIT_CAPACITY, (u8*)&elm, sizeof(ELM), NULL, NULL);
    //CHECK_FATAL(!set->buckets, "set bucket init failed");

    set->capacity = HASHMAP_INIT_CAPACITY;
    set->size = 0;
    set->elm_size = elm_size;

    set->hash_fn = hash_fn ? hash_fn : fnv1a_hash;
    set->elm_del_fn = elm_del;
    set->cmp_fn = cmp_fn ? cmp_fn : default_compare;

    return set;
}


void hashset_destroy(hashset *set)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!set->buckets, "set bucket is null");

    for (u32 i = 0; i < set->capacity; i++) {
        elm_destroy(set, (const ELM*)genVec_get_ptr(set->buckets, i));
    }

    genVec_destroy(set->buckets);

    free(set);
}


b8 hashset_insert(hashset* set, const u8* elm)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!elm, "elm is null");
    
    hashset_maybe_resize(set);

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(set, elm, &found, &tombstone);

    if (found) {
        return 1; // found
    }
    else {
        ELM e = {
            .elm = malloc(set->elm_size),
            .state = FILLED
        };

        CHECK_FATAL(!elm, "elm malloc failed");

        memcpy(e.elm, elm, set->elm_size);
        
        genVec_replace(set->buckets, slot, (u8*)&e);
        set->size++;

        return 0; // not found
    }
}

b8 hashset_remove(hashset* set, const u8* elm)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!elm, "elm is null");
    //if (set->size == 0) { return -1; }
    CHECK_WARN_RET(set->size == 0, -1, "can't remove from empty set");

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(set, elm, &found, &tombstone);

    if (found) {
        ELM* elm = (ELM*)genVec_get_ptr(set->buckets, slot);
        elm_destroy(set, elm);

        ELM newelm = {
            .elm = NULL,
            .state = TOMBSTONE
        };
        genVec_replace(set->buckets, slot, (u8*)&newelm);
        set->size--;

        hashset_maybe_resize(set);
        return 1;// found
    }
    else {
        return 0; // not found
    }
}

b8 hashset_has(const hashset* set, const u8* elm)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!elm, "set is null");

    b8 found = 0;
    int tombstone = -1;
    find_slot(set, elm, &found, &tombstone);

    return found;
}

void hashset_print(const hashset* set, genVec_print_fn elm_print)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!elm_print, "elm_print is null");

    printf("\t=========\n");

    for (u32 i = 0; i < set->capacity; i++) {

        const ELM* elm = (const ELM*)genVec_get_ptr(set->buckets, i);
        if (elm->state == FILLED) {
            printf("\t   ");
            elm_print(elm->elm);
            printf("\n");
        }
    }

    printf("\t=========\n");
}


