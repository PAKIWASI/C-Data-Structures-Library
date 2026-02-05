#include "gen_vector.h"

#include <stdlib.h>
#include <string.h>


#define GENVEC_MIN_CAPACITY 4


// Helper macros

// Get pointer to data array (handles both stack and heap storage)
#define GET_DATA(vec) ((vec)->svo ? (vec)->data.stack : (vec)->data.heap)
// get ptr to elm at index i
#define GET_PTR(vec, i) (GET_DATA((vec)) + ((u64)(i) * ((vec)->data_size)))
// get total_size in bytes for i elements
#define GET_SCALED(vec, i) ((u64)(i) * ((vec)->data_size))
// check if we need to grow vector
#define MAYBE_GROW(vec)                                                                   \
    do {                                                                                  \
        if (!vec->svo && !vec->data.heap) {                                               \
            genVec_grow(vec);                                                             \
        } else if (vec->size >= vec->capacity) {                                          \
            if (vec->svo) {                                                               \
                genVec_migrate_to_heap(vec, (u32)((float)vec->capacity * GENVEC_GROWTH)); \
            } else {                                                                      \
                genVec_grow(vec);                                                         \
            }                                                                             \
        }                                                                                 \
    } while (0)
// check if we need to shrink vector
#define MAYBE_SHRINK(vec)                                                               \
    do {                                                                                \
        if (!vec->svo && vec->size <= (u32)((float)vec->capacity * GENVEC_SHRINK_AT)) { \
            genVec_shrink(vec);                                                         \
        }                                                                               \
    } while (0)


//private functions

void genVec_grow(genVec* vec);
void genVec_shrink(genVec* vec);
void genVec_migrate_to_heap(genVec* vec, u32 new_capacity);


// API Implementation

genVec* genVec_init(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn)
{
    CHECK_FATAL(data_size == 0, "data_size can't be 0");

    genVec* vec = (genVec*)malloc(sizeof(genVec));
    CHECK_FATAL(!vec, "vec init failed");

    // Calculate how many elements can fit in SVO storage
    u32 svo_cap = GENVEC_SVO_CAPACITY(data_size);

    // Use SVO if requested capacity fits
    if (/*n > 0 &&*/ n <= svo_cap) {
        vec->svo      = true;
        vec->capacity = svo_cap; // in stack mode if size <= svo_cap
    } else {
        // Only allocate memory if n > 0, otherwise data can be NULL
        vec->data.heap = (n > 0) ? (u8*)malloc((size_t)data_size * n) : NULL;

        // Only check for allocation failure if we actually tried to allocate
        if (n > 0 && !vec->data.heap) {
            free(vec);
            FATAL("data init failed");
        }

        vec->svo      = false;
        vec->capacity = n;
    }

    vec->size      = 0;
    vec->data_size = data_size;

    vec->copy_fn = copy_fn;
    vec->move_fn = move_fn;
    vec->del_fn  = del_fn;

    return vec;
}


void genVec_init_stk(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn,
                     genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(data_size == 0, "data_size can't be 0");


    u32 svo_cap = GENVEC_SVO_CAPACITY(data_size);

    if (n <= svo_cap) {
        vec->svo      = true;
        vec->capacity = svo_cap;
    } else {
        // Only allocate memory if n > 0, otherwise data can be NULL
        vec->data.heap = (n > 0) ? (u8*)malloc((size_t)data_size * n) : NULL;
        CHECK_FATAL(n > 0 && !vec->data.heap, "data init failed");

        vec->svo      = false;
        vec->capacity = n;
    }

    vec->size      = 0;
    vec->data_size = data_size;
    vec->copy_fn   = copy_fn;
    vec->move_fn   = move_fn;
    vec->del_fn    = del_fn;
}

genVec* genVec_init_val(u32 n, const u8* val, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn,
                        genVec_delete_fn del_fn)
{
    CHECK_FATAL(!val, "val can't be null");
    CHECK_FATAL(n == 0, "cant init with val if n = 0");

    genVec* vec = genVec_init(n, data_size, copy_fn, move_fn, del_fn);

    vec->size = n; //capacity set to n in upper func

    for (u32 i = 0; i < n; i++) {
        if (copy_fn) {
            copy_fn(GET_PTR(vec, i), val);
        } else {
            memcpy(GET_PTR(vec, i), val, data_size);
        }
    }

    return vec;
}

void genVec_init_val_stk(u32 n, const u8* val, u16 data_size, 
        genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn, genVec* vec)
{
    CHECK_FATAL(!val, "val can't be null");
    CHECK_FATAL(n == 0, "cant init with val if n = 0");

    genVec_init_stk(n, data_size, copy_fn, move_fn, del_fn, vec);

    vec->size = n;

    for (u32 i = 0; i < n; i++) {
        if (copy_fn) {
            copy_fn(GET_PTR(vec, i), val);
        } else {
            memcpy(GET_PTR(vec, i), val, data_size);
        }
    }
}

void genVec_destroy(genVec* vec)
{
    genVec_destroy_stk(vec);

    free(vec);
}


void genVec_destroy_stk(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (!vec->svo && !vec->data.heap) {
        return;
    }

    if (vec->del_fn) {
        // Custom cleanup for each element
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }

    // Free heap data if in heap mode
    if (!vec->svo && vec->data.heap) {
        free(vec->data.heap);
        vec->data.heap = NULL;
    }
    // dont free vec as on stk (don't own memory)
}

void genVec_clear(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (vec->del_fn) { // owns resources
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }
    // doesn't free container
    vec->size = 0;
}

void genVec_reset(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (vec->del_fn) {
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }

    if (!vec->svo) {
        free(vec->data.heap);
        vec->data.heap = NULL;
        vec->capacity  = GENVEC_SVO_CAPACITY(vec->data_size);
        vec->svo       = true; // only time it's reset
    }

    vec->size = 0;
}


void genVec_reserve(genVec* vec, u32 new_capacity)
{
    CHECK_FATAL(!vec, "vec is null");

    // Only grow, never shrink with reserve
    if (new_capacity <= vec->capacity) {
        return;
    }

    if (vec->svo) {
        genVec_migrate_to_heap(vec, new_capacity); // cap is greater than curr cap
        return;
    }

    u8* new_data = realloc(vec->data.heap, GET_SCALED(vec, new_capacity));
    CHECK_FATAL(!new_data, "realloc failed");

    vec->data.heap = new_data;
    vec->capacity  = new_capacity;
}

void genVec_reserve_val(genVec* vec, u32 new_capacity, const u8* val)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!val, "val is null");
    CHECK_FATAL(new_capacity < vec->size, "new_capacity must be >= current size");

    genVec_reserve(vec, new_capacity);

    for (u32 i = vec->size; i < new_capacity; i++) {
        if (vec->copy_fn) {
            vec->copy_fn(GET_PTR(vec, i), val);
        } else {
            memcpy(GET_PTR(vec, i), val, vec->data_size);
        }
    }
    vec->size = new_capacity;
}


void genVec_push(genVec* vec, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");

    // Check if we need to allocate or grow
    MAYBE_GROW(vec);

    if (vec->copy_fn) {
        vec->copy_fn(GET_PTR(vec, vec->size), data);
    } else {
        memcpy(GET_PTR(vec, vec->size), data, vec->data_size);
    }

    vec->size++;
}


void genVec_push_move(genVec* vec, u8** data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(!*data, "*data is null");

    // Check if we need to allocate or grow
    MAYBE_GROW(vec);

    if (vec->move_fn) {
        vec->move_fn(GET_PTR(vec, vec->size), data);
    } else {
        // copy the pointer to resource
        memcpy(GET_PTR(vec, vec->size), *data, vec->data_size);
        *data = NULL; // now arr owns the resource
    }

    vec->size++;
}


// pop can't be a move operation (array is contiguos)
void genVec_pop(genVec* vec, u8* popped)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_WARN_RET(vec->size == 0, , "vec is empty");

    u8* last_elm = GET_PTR(vec, vec->size - 1);

    if (popped) {
        if (vec->copy_fn) {
            vec->copy_fn(popped, last_elm);
        } else {
            memcpy(popped, last_elm, vec->data_size);
        }
    }

    if (vec->del_fn) { // del func frees the resources owned by last_elm, but not ptr
        vec->del_fn(last_elm);
    }

    vec->size--; // set for re-write

    MAYBE_SHRINK(vec);
}


void genVec_get(const genVec* vec, u32 i, u8* out)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");
    CHECK_FATAL(!out, "need a valid out variable to get the element");

    if (vec->copy_fn) {
        vec->copy_fn(out, GET_PTR(vec, i));
    } else {
        memcpy(out, GET_PTR(vec, i), vec->data_size);
    }
}


const u8* genVec_get_ptr(const genVec* vec, u32 i)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");

    return GET_PTR(vec, i);
}


void genVec_for_each(genVec* vec, void (*for_each)(u8* elm))
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!for_each, "for_each function is null");

    for (u32 i = 0; i < vec->size; i++) {
        for_each(GET_PTR(vec, i));
    }
}


void genVec_insert(genVec* vec, u32 i, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(i > vec->size, "index out of bounds");

    if (i == vec->size) {
        genVec_push(vec, data);
        return;
    }

    MAYBE_GROW(vec);

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;
    // the place where we want to insert
    u8* src = GET_PTR(vec, i);

    // Shift elements right by one unit
    u8* dest = GET_PTR(vec, i + 1);
    memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // Use memmove for overlapping regions

    //src pos is now free to insert (it's data copied to next location)
    if (vec->copy_fn) {
        vec->copy_fn(src, data);
    } else {
        memcpy(src, data, vec->data_size);
    }

    vec->size++;
}

void genVec_insert_move(genVec* vec, u32 i, u8** data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(!*data, "*data is null");
    CHECK_FATAL(i > vec->size, "index out of bounds");

    if (i == vec->size) {
        genVec_push_move(vec, data);
        return;
    }

    MAYBE_GROW(vec);

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;
    // the place where we want to insert
    u8* src = GET_PTR(vec, i);

    // Shift elements right by one unit
    u8* dest = GET_PTR(vec, i + 1);
    memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // Use memmove for overlapping regions


    if (vec->move_fn) {
        vec->move_fn(src, data);
    } else {
        memcpy(src, *data, vec->data_size);
        *data = NULL;
    }

    vec->size++;
}


void genVec_insert_multi(genVec* vec, u32 i, const u8* data, u32 num_data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(num_data == 0, "num_data can't be 0");
    CHECK_FATAL(i > vec->size, "index out of bounds");

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;

    vec->size += num_data; // no of new elements in chunk

    genVec_reserve(vec, vec->size); // reserve with new size

    // the place where we want to insert
    u8* src = GET_PTR(vec, i);
    if (elements_to_shift > 0) {
        // Shift elements right by num_data units to right
        u8* dest = GET_PTR(vec, i + num_data);

        memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // using memmove for overlapping regions
    }

    //src pos is now free to insert (it's data copied to next location)
    if (vec->copy_fn) {
        for (u32 j = 0; j < num_data; j++) {
            vec->copy_fn(GET_PTR(vec, j + i), (data + (size_t)(j * vec->data_size)));
        }
    } else {
        memcpy(src, data, GET_SCALED(vec, num_data));
    }
}

void genVec_insert_multi_move(genVec* vec, u32 i, u8** data, u32 num_data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(!*data, "*data is null");
    CHECK_FATAL(num_data == 0, "num_data can't be 0");
    CHECK_FATAL(i > vec->size, "index out of bounds");

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;

    vec->size += num_data; // no of new elements in chunk

    genVec_reserve(vec, vec->size); // reserve with new size

    // the place where we want to insert
    u8* src = GET_PTR(vec, i);
    if (elements_to_shift > 0) {
        // Shift elements right by num_data units to right
        u8* dest = GET_PTR(vec, i + num_data);

        memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // using memmove for overlapping regions
    }

    //src pos is now free to insert (it's data copied to next location)
    // Move entire contiguous block at once
    memcpy(src, *data, GET_SCALED(vec, num_data));
    *data = NULL; // Transfer ownership
}


void genVec_remove(genVec* vec, u32 i, u8* out)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");

    if (out) {
        if (vec->copy_fn) {
            vec->copy_fn(out, GET_PTR(vec, i));
        } else {
            memcpy(out, GET_PTR(vec, i), vec->data_size);
        }
    }

    if (vec->del_fn) {
        vec->del_fn(GET_PTR(vec, i));
    }
    // Calculate the number of elements to shift
    u32 elements_to_shift = vec->size - i - 1;

    if (elements_to_shift > 0) {
        // Shift elements left to overwrite the deleted element
        u8* dest = GET_PTR(vec, i);
        u8* src  = GET_PTR(vec, i + 1);

        memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // Use memmove for overlapping regions
    }

    vec->size--;

    MAYBE_SHRINK(vec);
}


void genVec_remove_range(genVec* vec, u32 l, u32 r)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(l >= vec->size, "index out of range");
    CHECK_FATAL(l > r, "invalid range");

    if (r >= vec->size) {
        r = vec->size - 1;
    }

    if (vec->del_fn) {
        for (u32 i = l; i <= r; i++) {
            u8* elm = GET_PTR(vec, i);
            vec->del_fn(elm);
        }
    }

    u32 elms_to_shift = vec->size - (r + 1);

    // move from r + 1 to l
    u8* dest = GET_PTR(vec, l);
    u8* src  = GET_PTR(vec, r + 1);
    memmove(dest, src, GET_SCALED(vec, elms_to_shift)); // Use memmove for overlapping regions

    vec->size -= (r - l + 1);

    MAYBE_SHRINK(vec);
}


void genVec_replace(genVec* vec, u32 i, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");
    CHECK_FATAL(!data, "need a valid data variable");

    u8* to_replace = GET_PTR(vec, i);

    if (vec->del_fn) {
        vec->del_fn(to_replace);
    }

    if (vec->copy_fn) {
        vec->copy_fn(to_replace, data);
    } else {
        memcpy(to_replace, data, vec->data_size);
    }
}


void genVec_replace_move(genVec* vec, u32 i, u8** data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");
    CHECK_FATAL(!data, "need a valid data variable");
    CHECK_FATAL(!*data, "need a valid *data variable");

    u8* to_replace = GET_PTR(vec, i);

    if (vec->del_fn) {
        vec->del_fn(to_replace);
    }

    if (vec->move_fn) {
        vec->move_fn(to_replace, data);
    } else {
        memcpy(to_replace, *data, vec->data_size);
        *data = NULL;
    }
}


const u8* genVec_front(const genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(vec->size == 0, "vec is empty");

    return GET_DATA(vec);
}


const u8* genVec_back(const genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(vec->size == 0, "vec is empty");

    return GET_PTR(vec, vec->size - 1);
}


void genVec_print(const genVec* vec, genVec_print_fn fn)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!fn, "print func is null");

    printf("[ ");
    for (u32 i = 0; i < vec->size; i++) {
        fn(GET_PTR(vec, i));
        printf(" ");
    }
    printf("]\n");

    printf("Size: %u\n", vec->size);
    printf("Capacity: %u\n", vec->capacity);
}


void genVec_copy(genVec* dest, const genVec* src)
{
    CHECK_FATAL(!dest, "dest is null");
    CHECK_FATAL(!src, "src is null");

    if (dest == src) {
        return;
    }

    // if data ptr is null, no op
    genVec_destroy_stk(dest);

    // copy all fields
    memcpy(dest, src, sizeof(genVec));

    if (src->svo) { // we already have all elements
        return;
    }

    // malloc data ptr
    dest->data.heap = (u8*)malloc(GET_SCALED(src, src->capacity));

    // Copy elements
    if (src->copy_fn) {
        for (u32 i = 0; i < src->size; i++) {
            src->copy_fn(GET_PTR(dest, i), GET_PTR(src, i));
        }
    } else {
        memcpy(dest->data.heap, src->data.heap, GET_SCALED(src, src->size));
    }
}


void genVec_move(genVec* dest, genVec** src)
{
    CHECK_FATAL(!src, "src is null");
    CHECK_FATAL(!*src, "*src is null");
    CHECK_FATAL(!dest, "dest is null");
    // CHECK_FATAL((*src)->svo, "can't move a SVO (stack) vec");

    if (dest == *src) {
        *src = NULL;
        return;
    }

    // Transfer all fields from src to dest
    memcpy(dest, *src, sizeof(genVec));

    if ((*src)->svo) {
        (*src)->size = 0; // can't acces data anymore
        return;
    }

    // Null out src's data pointer so it doesn't get freed
    (*src)->data.heap = NULL;

    // Free src if it was heap-allocated
    // This only frees the genVec struct itself, not the data
    // (which was transferred to dest)
    free(*src);
    *src = NULL;
}


void genVec_grow(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    u32 new_cap;
    if (vec->capacity < GENVEC_MIN_CAPACITY) {
        new_cap = vec->capacity + 1;
    } else {
        new_cap = (u32)((float)vec->capacity * GENVEC_GROWTH);
        if (new_cap <= vec->capacity) { // Ensure at least +1 growth
            new_cap = vec->capacity + 1;
        }
    }

    u8* new_data = realloc(vec->data.heap, GET_SCALED(vec, new_cap));
    CHECK_FATAL(!new_data, "realloc failed");

    vec->data.heap = new_data;
    vec->capacity  = new_cap;
}


void genVec_shrink(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    u32 reduced_cap = (u32)((float)vec->capacity * GENVEC_SHRINK_BY);
    if (reduced_cap < vec->size || reduced_cap == 0) {
        return;
    }

    u8* new_data = realloc(vec->data.heap, GET_SCALED(vec, reduced_cap));
    if (!new_data) {
        CHECK_WARN_RET(1, , "data realloc failed");
        return; // Keep original allocation
    }

    vec->data.heap = new_data;
    vec->capacity  = reduced_cap;
}

void genVec_migrate_to_heap(genVec* vec, u32 new_capacity)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!vec->svo, "vec already on heap");
    CHECK_FATAL(new_capacity <= vec->capacity, "new_capacity must be greater than old cap");

    u8* heap_data = (u8*)malloc(GET_SCALED(vec, new_capacity));
    CHECK_FATAL(!heap_data, "heap data malloc failed");

    memcpy(heap_data, vec->data.stack, GET_SCALED(vec, vec->size));

    vec->data.heap = heap_data;
    vec->capacity  = new_capacity;
    vec->svo       = false; // once set to false, can't be set to true again
}


