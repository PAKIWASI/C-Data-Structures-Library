#include "hashset.h"
#include "common.h"
#include <string.h>



typedef struct {
    u8* elm;
    STATE state;
} ELM;

/*
====================ELM HANDLERS====================
*/

static void elm_destroy(hashset* set, ELM* elm)
{
    CHECK_FATAL(!elm, "ELM is null");

    if (elm->elm) {
        if (set->del_fn) {
            set->del_fn(elm->elm);
        }
        free(elm->elm);
    }
}

/*
====================PRIVATE FUNCTIONS====================
*/

// CRITICAL: find_slot ALWAYS takes const u8* (not u8**)
static u32 find_slot(const hashset* set, const u8* elm,
                        b8* found, int* tombstone)
{
    u32 index = set->hash_fn(elm, set->elm_size) % set->capacity;

    *found = 0;
    *tombstone = -1;

    for (u32 x = 0; x < set->capacity; x++) {
        u32 i = (index + x) % set->capacity;
        const ELM* e = (const ELM*)genVec_get_ptr(set->buckets, i);

        switch (e->state) {
            case EMPTY:
                return i;
            case FILLED:
                if (set->cmp_fn(e->elm, elm, set->elm_size) == 0) 
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

static void hashset_resize(hashset* set, u32 new_capacity) 
{
    if (new_capacity <= HASHMAP_INIT_CAPACITY) {
        new_capacity = HASHMAP_INIT_CAPACITY;
    }

    genVec* old_vec = set->buckets;

    ELM elm = { .elm = NULL, .state = EMPTY };
    set->buckets = genVec_init_val(new_capacity, (u8*)&elm, sizeof(ELM), NULL, NULL, NULL);

    set->capacity = new_capacity;
    set->size = 0;

    for (u32 i = 0; i < old_vec->capacity; i++) {
        const ELM* old_elm = (const ELM*)genVec_get_ptr(old_vec, i);
        
        if (old_elm->state == FILLED) {
            b8 found = 0;
            int tombstone = -1;
            u32 slot = find_slot(set, old_elm->elm, &found, &tombstone);

            ELM new_elm = {
                .elm = old_elm->elm,
                .state = FILLED
            };

            genVec_replace(set->buckets, slot, (u8*)&new_elm);
            set->size++;
        }
    }

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
    else if (load_factor < LOAD_FACTOR_SHRINK && set->capacity > HASHMAP_INIT_CAPACITY) 
    {
        u32 new_cap = prev_prime(set->capacity);
        if (new_cap >= HASHMAP_INIT_CAPACITY) {
            hashset_resize(set, new_cap);
        }
    }
}

/*
====================PUBLIC FUNCTIONS====================
*/

hashset* hashset_create(const u16 elm_size, custom_hash_fn hash_fn, compare_fn cmp_fn, 
                         copy_fn copy_fn, move_fn move_fn, delete_fn del_fn)
{
    CHECK_FATAL(elm_size == 0, "elm size can't be 0");

    hashset* set = malloc(sizeof(hashset));
    CHECK_FATAL(!set, "set malloc failed");

    ELM elm = { .elm = NULL, .state = EMPTY };
    set->buckets = genVec_init_val(HASHMAP_INIT_CAPACITY, (u8*)&elm, sizeof(ELM), NULL, NULL, NULL);

    set->capacity = HASHMAP_INIT_CAPACITY;
    set->size = 0;
    set->elm_size = elm_size;

    set->hash_fn = hash_fn ? hash_fn : fnv1a_hash;
    set->cmp_fn = cmp_fn ? cmp_fn : default_compare;

    set->copy_fn = copy_fn;
    set->move_fn = move_fn;
    set->del_fn  = del_fn;

    return set;
}

void hashset_destroy(hashset *set)
{
    CHECK_FATAL(!set, "set is null");

    for (u32 i = 0; i < set->capacity; i++) {
        ELM* e = (ELM*)genVec_get_ptr(set->buckets, i);
        if (e->state == FILLED) {
            elm_destroy(set, e);
        }
    }

    genVec_destroy(set->buckets);
    free(set);
}

// COPY semantics
b8 hashset_insert(hashset* set, const u8* elm)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!elm, "elm is null");
    
    hashset_maybe_resize(set);

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(set, elm, &found, &tombstone);

    if (found) {
        return 1; // already exists
    }
    
    // New element - insert
    ELM e = {
        .elm = malloc(set->elm_size),
        .state = FILLED
    };
    CHECK_FATAL(!e.elm, "elm malloc failed");

    if (set->copy_fn) {
        set->copy_fn(e.elm, elm);
    } else {
        memcpy(e.elm, elm, set->elm_size);
    }
    
    genVec_replace(set->buckets, slot, (u8*)&e);
    set->size++;

    return 0; // inserted
}

// MOVE semantics
b8 hashset_insert_move(hashset* set, u8** elm)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!elm, "elm is null");
    CHECK_FATAL(!*elm, "*elm is null");
    
    hashset_maybe_resize(set);

    b8 found = 0;
    int tombstone = -1;
    // IMPORTANT: Dereference *elm to pass u8* to find_slot
    u32 slot = find_slot(set, *elm, &found, &tombstone);

    if (found) {
        // Element already exists - clean up the passed element
        if (set->del_fn) {
            set->del_fn(*elm);
        }
        free(*elm);
        *elm = NULL;
        return 1; // already exists
    }

    // New element - insert
    ELM e = {
        .elm = malloc(set->elm_size),
        .state = FILLED
    };
    CHECK_FATAL(!e.elm, "elm malloc failed");

    if (set->move_fn) {
        set->move_fn(e.elm, elm);
    } else {
        memcpy(e.elm, *elm, set->elm_size);
        *elm = NULL;
    }
    
    genVec_replace(set->buckets, slot, (u8*)&e);
    set->size++;

    return 0; // inserted
}

b8 hashset_remove(hashset* set, const u8* elm)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!elm, "elm is null");
    CHECK_WARN_RET(set->size == 0, (b8)-1, "can't remove from empty set");

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(set, elm, &found, &tombstone);

    if (found) {
        ELM* e = (ELM*)genVec_get_ptr(set->buckets, slot);
        elm_destroy(set, e);

        ELM newelm = { .elm = NULL, .state = TOMBSTONE };
        genVec_replace(set->buckets, slot, (u8*)&newelm);
        set->size--;

        hashset_maybe_resize(set);
        return 1; // found
    }

    return 0; // not found
}

b8 hashset_has(const hashset* set, const u8* elm)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!elm, "elm is null");

    b8 found = 0;
    int tombstone = -1;
    find_slot(set, elm, &found, &tombstone);

    return found;
}

void hashset_print(const hashset* set, genVec_print_fn print_fn)
{
    CHECK_FATAL(!set, "set is null");
    CHECK_FATAL(!print_fn, "elm_print is null");

    printf("\t=========\n");
    printf("\tSize: %u / Capacity: %u\n", set->size, set->capacity);
    printf("\t=========\n");

    for (u32 i = 0; i < set->capacity; i++) {
        const ELM* elm = (const ELM*)genVec_get_ptr(set->buckets, i);
        if (elm->state == FILLED) {
            printf("\t   ");
            print_fn(elm->elm);
            printf("\n");
        }
    }

    printf("\t=========\n");
}


