#include "gen_vector.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define GROWTH      1.5 
#define SHRINK_AT   0.25
#define SHRINK_BY   0.5


#define GET_PTR(vec, i)     ((vec->data) + ((size_t)(i) * (vec->data_size)))
#define GET_SCALED(vec, i)  ((size_t)(i) * (vec->data_size))


//private functions
void genVec_grow(genVec* vec);
void genVec_shrink(genVec* vec);


genVec* genVec_init(u32 n, u16 data_size, genVec_delete_fn del_fn) 
{
    if (data_size == 0) { 
        ERROR("data_size can't be 0");
        return NULL; 
    }

    genVec* vec = malloc(sizeof(genVec));
    if (!vec) { 
        ERROR("vec init failed");
        return NULL; 
    }

    // Only allocate memory if n > 0, otherwise data can be NULL
    vec->data = (n > 0) ? malloc((size_t)data_size * n) : NULL;
    
    // Only check for allocation failure if we actually tried to allocate
    if (n > 0 && !vec->data) {
        free(vec);
        ERROR("data init failed");
        return NULL;
    }
    
    vec->size = 0;
    vec->capacity = n;
    vec->data_size = data_size;

    vec->del_fn = del_fn; // NULL if no del_fn

    return vec;
}

void genVec_init_stk(u32 n, u16 data_size, genVec_delete_fn del_fn, genVec* vec)
{
    if (data_size == 0) { 
        ERROR("data_size can't be 0");
        return; 
    }
    if (!vec) { 
        ERROR("vec null");
        return; 
    }

    // Only allocate memory if n > 0, otherwise data can be NULL
    vec->data = (n > 0) ? malloc((size_t)data_size * n) : NULL;
    if (n > 0 && !vec->data) {
        ERROR("data init failed");
        return;
    }
    
    vec->size = 0;
    vec->capacity = n;
    vec->data_size = data_size;
    vec->del_fn = del_fn; // NULL if no del_fn
}

genVec* genVec_init_val(u32 n, const u8* val, u16 data_size, genVec_delete_fn del_fn) 
{
    if (val == NULL) {
        ERROR("val can't be null");
        return NULL;
    }

    genVec* vec = genVec_init(n, data_size, del_fn);
    if (!vec) { return NULL; }

    if (n == 0) {
        ERROR("cant init with val if n = 0");
        return vec;
    }

    vec->size = n;  //capacity set to n in upper func 

    for (u32 i = 0; i < n; i++) {
        memcpy(GET_PTR(vec, i), val, data_size);
    }

    return vec;
}

void genVec_destroy(genVec* vec) 
{
    if (!vec) { return; }
    
    if (vec->del_fn) {
        // Custom cleanup for each element
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }
    
    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }
    free(vec);
}

void genVec_destroy_stk(genVec* vec)
{
    if (!vec) { return; }
    
    if (vec->del_fn) {
        // Custom cleanup for each element
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }
    
    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }
    // dont free vec as on stk
}

void genVec_clear(genVec* vec) {
    if (!vec) {
        ERROR("vector is null");
        return;
    }

    if (vec->del_fn) {
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }

    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }

    vec->size = 0;
    vec->capacity = 0;
}

void genVec_reserve(genVec* vec, u32 new_capacity) 
{
    if (!vec) {
        ERROR("vec is null");
        return;
    }
    
    // Only grow, never shrink with reserve
    if (new_capacity <= vec->capacity) {
        return;
    }
    
    u8* new_data = realloc(vec->data, GET_SCALED(vec, new_capacity));
    if (!new_data) {
        ERROR("realloc failed");
        return;
    }
    
    vec->data = new_data;
    vec->capacity = new_capacity;
}

void genVec_reserve_val(genVec* vec, u32 new_capacrity, const u8* val)
{
    if (!vec) {
        ERROR("vec is null");
        return;
    }

    genVec_reserve(vec, new_capacrity);
    
    // TODO: check the bounds
    for (u32 i = vec->size; i < new_capacrity; i++) {
        genVec_push(vec, val);
    }

    vec->size = new_capacrity;
}

void genVec_push(genVec* vec, const u8* data) 
{
    if (!vec) {
        ERROR("vec is null");
        return;
    }

    // Check if we need to allocate or grow
    if (vec->size >= vec->capacity || !vec->data) 
        { genVec_grow(vec); }

    // If data is still NULL after grow, we have a problem
    if (!vec->data) {
        ERROR("data allocation failed");
        return;
    }

   // u8* next_to_last = vec->data + ((size_t)vec->size * vec->data_size); 
    memcpy(GET_PTR(vec, vec->size), data, vec->data_size);

    vec->size++;
}

void genVec_pop(genVec* vec, u8* popped) 
{
    if (!vec) {
        ERROR("vec is null");
        return;
    }
    if (vec->size == 0) {
        ERROR("vec is empty");
        return;
    }
    
    u8* last_elm = GET_PTR(vec, vec->size - 1);
    if (popped) { //only if buffer provided
        memcpy(popped, last_elm, vec->data_size);
    }
    else if (vec->del_fn) {
        vec->del_fn(last_elm);  // (if it's a pointer, like String*)
    }

    vec->size--;    // set for re-write

    if (vec->size <= (u32)((double)vec->capacity * SHRINK_AT)) 
        { genVec_shrink(vec); }
}

void genVec_get(const genVec* vec, u32 i, u8* out) 
{
    if (!vec) {
        printf("get: vec is null\n");
        ERROR("vec is null");
        return;
    }
    if (i >= vec->size) {
        printf("get: index of of bounds\n");
        return;
    }
    if (!out) {
        printf("get: need a valid out variable to get the element\n");
        return;
    }

    memcpy(out, GET_PTR(vec, i), vec->data_size);
}

const u8* genVec_get_ptr(const genVec* vec, u32 i)
{
    if (!vec) {
        printf("get ptr: vec is null\n");
        return NULL;
    }
    if (i >= vec->size) {
        printf("get ptr: index out of bounds\n");
        return NULL;
    }

    return GET_PTR(vec, i);
}

void genVec_insert(genVec* vec, u32 i, const u8* data)
{
    if (!vec || !data) {
        printf("insert: vec or data or vec-data is null\n");
        return;
    }
    if (i > vec->size) {
        printf("insert: index out of bounds\n");
        return;
    }
    if (i == vec->size) {
        genVec_push(vec, data);
        return;
    }

    // Check if we need to allocate or grow
    if (vec->size >= vec->capacity || !vec->data) 
        { genVec_grow(vec); }

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;
    // the place where we want to insert
    u8* src = GET_PTR(vec, i);

    // Shift elements right by one unit
    u8* dest = GET_PTR(vec, i + 1);
    memmove(dest, src, GET_SCALED(vec, elements_to_shift));  // Use memmove for overlapping regions

    //src pos is now free to insert (it's data copied to next location)
    memcpy(src, data, vec->data_size);

    vec->size++;  
}


void genVec_insert_multi(genVec* vec, u32 i, const u8* data, u32 num_data)
{
    if (!vec || !data || num_data == 0) 
    {
        printf("insertM: vec or data or vec-data is null\n");
        return;
    }
    if (i > vec->size) {
        printf("insertM: index out of bounds\n");
        return;
    }

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;

    vec->size += num_data; // no of new elements in chunk

    genVec_reserve(vec, vec->size); // reserve with new size
    if (!vec->data) {
        printf("insertM: genvec reserve failed\n");
        vec->size -= num_data;
        return;
    }

    // the place where we want to insert
    u8* src = GET_PTR(vec, i);
    if (elements_to_shift > 0) {
        // Shift elements right by num_data units to right 
        u8* dest = GET_PTR(vec, i + num_data);

        memmove(dest, src, GET_SCALED(vec, elements_to_shift));  // using memmove for overlapping regions
    }

    //src pos is now free to insert (it's data copied to next location)
    memcpy(src, data, GET_SCALED(vec, num_data));
}

void genVec_remove(genVec* vec, u32 i) 
{
    if (!vec) {
        printf("remove: vec is null\n");
        return;
    }
    if (i >= vec->size) {
        printf("remove: index out of bounds\n");
        return;
    }

    if (vec->del_fn) {
        u8* element = GET_PTR(vec, i);
        vec->del_fn(element);
    }
        // Calculate the number of elements to shift
    u32 elements_to_shift = vec->size - i - 1;
    
    if (elements_to_shift > 0) {
        // Shift elements left to overwrite the deleted element
        u8* dest = GET_PTR(vec, i);
        u8* src = GET_PTR(vec, i + 1);
        
        memmove(dest, src, GET_SCALED(vec, elements_to_shift));  // Use memmove for overlapping regions
    }

    vec->size--;
    
    if (vec->size <= (u32)((double)vec->capacity * SHRINK_AT)) 
        { genVec_shrink(vec); }
}

void genVec_remove_range(genVec* vec, u32 l, u32 r)
{
    if (!vec) {
        printf("remove range: vec is null\n");
        return;
    }

    if (l >= vec->size) {
        printf("remove range: index out of range\n");
        return;
    }

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
    u8* src = GET_PTR(vec, r + 1);
    memmove(dest, src, GET_SCALED(vec, elms_to_shift));  // Use memmove for overlapping regions

    vec->size -= (r - l + 1);
}


void genVec_replace(genVec* vec, u32 i, const u8* data) 
{
    if (!vec) {
        printf("replace: vec is null\n");
        return;
    } 
    if (i >= vec->size) {
        printf("replace: index of of bounds\n");
        return;
    }
    if (!data) {
        printf("replace: need a valid data variable\n");
        return;
    }   

    u8* to_replace = GET_PTR(vec, i);

    if (vec->del_fn) {
        vec->del_fn(to_replace);
    }

    memcpy(to_replace, data, vec->data_size);
}

u8* genVec_front(const genVec* vec) 
{
    if (!vec) {
        printf("front: vec is null\n");
        return NULL;
    }
    if (vec->size == 0) {
        printf("front: vec is empty\n");
        return NULL;
    }
    
    return vec->data;
}


u8* genVec_back(const genVec* vec) 
{
    if (!vec) {
        printf("back: vec is null\n");
        return NULL;
    }
    if (vec->size == 0) {
        printf("back: vec is empty\n");
        return NULL;
    }
    
    //return vec->data + ((size_t)(vec->size - 1) * vec->data_size);
    return GET_PTR(vec, vec->size - 1);
}


void genVec_print(const genVec* vec, genVec_print_fn fn) 
{ 
    if (!vec) {
        printf("print: vec is null\n");
        return;
    }
    if (!fn) {
        printf("print: print func is null\n");
        return; 
    }

    printf("[ ");
    for (u32 i = 0; i < vec->size; i++) {
        u8* element = GET_PTR(vec, i);
        fn(element); 
        printf(" ");
    }
    printf("]\n");

    printf("Size: %u\n", vec->size);
    printf("Capacity: %u\n", vec->capacity);
}

void genVec_copy(genVec* dest, const genVec* src, genVec_copy_fn copy_fn)
{
    if (!dest || !src) {
        printf("copy: parameters null\n");
        return;
    }

    if (dest->data_size != src->data_size) {
        printf("copy: dest and src vec's data_size's don't match\n");
        return;
    }

    if (dest->capacity < src->size) {
        genVec_reserve(dest, src->size);
    }

    if (copy_fn) {
        for (u32 i = 0; i < src->size; i++) {
            copy_fn(GET_PTR(dest, i), GET_PTR(src, i));
        }
        dest->size = src->size;
    } else {
        memcpy(dest->data, src->data, GET_SCALED(src, src->size));
        dest->size = src->size;
    }
}

void genVec_grow(genVec* vec) 
{
    if (!vec) {
        printf("grow: vec is null\n");
        return;
    }

    u32 new_cap;
    if (vec->capacity < 4) { new_cap = vec->capacity + 1; }
    else {
        new_cap = (u32)((double)vec->capacity * GROWTH); 
    }

    u8* new_data = realloc(vec->data, GET_SCALED(vec, new_cap));
    if (!new_data) { 
        printf("grow: realloc failed\n");
        return;
    }

    vec->data = new_data;
    vec->capacity = new_cap;
}


void genVec_shrink(genVec* vec) 
{
    if (!vec) {
        printf("shrink: vec is null\n ");
        return;
    }

    u32 reduced_cap = (u32)((double)vec->capacity * SHRINK_BY);
    if (reduced_cap < vec->size || reduced_cap == 0) { return; }

    u8* new_data = realloc(vec->data, GET_SCALED(vec, reduced_cap));
    if (!new_data) {
        printf("shrink: realloc failed\n");
        return;
    }

    vec->data = new_data;
    vec->capacity = reduced_cap;
}


