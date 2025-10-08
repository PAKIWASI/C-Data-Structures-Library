#include "gen_vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define GROWTH 1.5 
#define SHRINK_AT 0.25
#define SHRINK_BY 0.5

//private functions
void genVec_grow(genVec* vec);
void genVec_shrink(genVec* vec);


genVec* genVec_init(size_t n, size_t data_size, genVec_delete_fn del_fn) {
    if (data_size == 0) { 
        printf("init: data_size can't be 0\n");
        return NULL; 
    }

    genVec* vec = malloc(sizeof(genVec));
    if (!vec) { 
        printf("init: vec init failed\n");
        return NULL; 
    }

    // Only allocate memory if n > 0, otherwise data can be NULL
    vec->data = (n > 0) ? malloc(data_size * n) : NULL;
    
    // Only check for allocation failure if we actually tried to allocate
    if (n > 0 && !vec->data) {
        free(vec);
        printf("vec init: data init failed\n");
        return NULL;
    }
    
    vec->size = 0;
    vec->capacity = n;
    vec->data_size = data_size;

    vec->del_fn = del_fn; // NULL if no del_fn

    return vec;
}

genVec* genVec_init_val(size_t n, const u8* val, size_t data_size, genVec_delete_fn del_fn) 
{
    if (val == NULL) {
        printf("val can't be null\n");
        return NULL;
    }

    genVec* vec = genVec_init(n, data_size, del_fn);
    if (!vec) { return NULL; }

    if (n == 0) {
        printf("cant init with val if n = 0\n");
        return vec;
    }

    vec->size = n;  //capacity set to n in upper func 

    for (size_t i = 0; i < n; i++) {
        memcpy(vec->data + (i * data_size), val, data_size);
    }

    return vec;
}

void genVec_destroy(genVec* vec) {
    if (!vec) {
        printf("del: vector is null\n");
        return;
    }
    
    if (vec->del_fn) {
        // Custom cleanup for each element
        for (size_t i = 0; i < vec->size; i++) {
            u8* element = vec->data + (i * vec->data_size);
            vec->del_fn(element);
        }
    }
    
    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }
    free(vec);
}

void genVec_clear(genVec* vec) {
    if (!vec) {
        printf("clear: vector is null\n");
        return;
    }

    if (vec->del_fn) {
        for (size_t i = 0; i < vec->size; i++) {
        u8* element = vec->data + (i * vec->data_size);
        vec->del_fn(element);
        }
    }

    if (vec->data) {
        free(vec->data);
        vec->data = NULL;
    }

    vec->size = 0;
    vec->capacity = 0;
}

void genVec_reserve(genVec* vec, size_t new_capacity) 
{
    if (!vec) {
        printf("reserve: vec is null\n");
        return;
    }
    
    // Only grow, never shrink with reserve
    if (new_capacity <= vec->capacity) {
        return;
    }
    
    u8* new_data = realloc(vec->data, new_capacity * vec->data_size);
    if (!new_data) {
        printf("reserve: realloc failed\n");
        return;
    }
    
    vec->data = new_data;
    vec->capacity = new_capacity;
}

void genVec_push(genVec* vec, const u8* data) 
{
    if (!vec) {
        printf("push: vec is null\n");
        return;
    }

    // Check if we need to allocate or grow
    if (vec->size >= vec->capacity || !vec->data) 
        { genVec_grow(vec); }

    // If data is still NULL after grow, we have a problem
    if (!vec->data) {
        printf("push: data allocation failed\n");
        return;
    }

    u8* next_to_last = vec->data + (vec->size * vec->data_size); 
    memcpy(next_to_last, data, vec->data_size);

    vec->size++;
}

int genVec_pop(genVec* vec, u8* popped) {
    if (!vec) {
        printf("pop: vec is null\n");
        return -1;
    }
    if (vec->size == 0) {
        printf("pop: vec is empty\n");
        return -1;
    }
    
    u8* last_elm = vec->data + ((vec->size - 1) * vec->data_size);
    if (popped) { //only if buffer provided
        memcpy(popped, last_elm, vec->data_size);
    }
    else if (vec->del_fn) {
        vec->del_fn(last_elm);  // (if it's a pointer, like String*)
    }

    vec->size--;    // set for re-write

    if (vec->size <= (size_t)((double)vec->capacity * SHRINK_AT)) 
        { genVec_shrink(vec); }

    return 0;
}

void genVec_get(const genVec* vec, size_t i, u8* out) {
    if (!vec) {
        printf("get: vec is null\n");
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

    u8* elm = vec->data + (i * vec->data_size);
    memcpy(out, elm, vec->data_size);
}

void genVec_insert(genVec* vec, size_t i, const u8* data)
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
    size_t elements_to_shift = vec->size - i;
    // the place where we want to insert
    u8* src = vec->data + (i * vec->data_size);

    // Shift elements right by one unit
    u8* dest = vec->data + ((i + 1) * vec->data_size);
    memmove(dest, src, elements_to_shift * vec->data_size);  // Use memmove for overlapping regions

    //src pos is now free to insert (it's data copied to next location)
    memcpy(src, data, vec->data_size);

    vec->size++;  
}


void genVec_insert_multi(genVec* vec, size_t i, const u8* data, size_t num_data)
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
    size_t elements_to_shift = vec->size - i;

    vec->size += num_data;

    genVec_reserve(vec, vec->size);
    if (!vec->data) {
        printf("insertM: genvec reserve failed\n");
        vec->size -= num_data;
        return;
    }

    // the place where we want to insert
    u8* src = vec->data + (i * vec->data_size);
    if (elements_to_shift > 0) {
        // Shift elements right by num_data units to right 
        u8* dest = vec->data + ((i + num_data) * vec->data_size);

        memmove(dest, src, elements_to_shift * vec->data_size);  // using memmove for overlapping regions
    }

    //src pos is now free to insert (it's data copied to next location)
    memcpy(src, data, num_data * vec->data_size);
}

void genVec_remove(genVec* vec, size_t i) {
    if (!vec) {
        printf("remove: vec is null\n");
        return;
    }
    if (i >= vec->size) {
        printf("remove: index out of bounds\n");
        return;
    }

    if (vec->del_fn) {
        u8* element = vec->data + (i * vec->data_size);
        vec->del_fn(element);
    }
        // Calculate the number of elements to shift
    size_t elements_to_shift = vec->size - i - 1;
    
    if (elements_to_shift > 0) {
        // Shift elements left to overwrite the deleted element
        u8* dest = vec->data + (i * vec->data_size);
        u8* src = vec->data + ((i + 1) * vec->data_size);
        
        memmove(dest, src, elements_to_shift * vec->data_size);  // Use memmove for overlapping regions
    }

    vec->size--;
    
    if (vec->size <= (size_t)((double)vec->capacity * SHRINK_AT)) 
        { genVec_shrink(vec); }
}



void genVec_replace(genVec* vec, size_t i, const u8* data) {
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

    u8* to_replace = vec->data + (i * vec->data_size); 

    if (vec->del_fn) {
        vec->del_fn(to_replace);
    }

    memcpy(to_replace, data, vec->data_size);
}

void genVec_front(const genVec* vec, u8* out) {
    if (!vec) {
        printf("front: vec is null\n");
        return;
    }
    if (vec->size == 0) {
        printf("front: vec is empty\n");
        return;
    }
    
    memcpy(out, vec->data, vec->data_size);
}


void genVec_back(const genVec* vec, u8* out) {
    if (!vec) {
        printf("back: vec is null\n");
        return;
    }
    if (vec->size == 0) {
        printf("back: vec is empty\n");
        return;
    }
    
    u8* last_elm = vec->data + ((vec->size - 1) * vec->data_size);
    memcpy(out, last_elm, vec->data_size);
}



// this is a shallow copy if elements are pointers
genVec* genVec_copy(genVec* src) {
    if (!src) {
        printf("copy: src is null\n");
        return NULL;
    }

    genVec* vec = genVec_init(src->size, src->data_size, src->del_fn);
    if (!vec) {
        printf("copy: genVec init failed\n");
        return NULL;
    }
    if (src->size == 0) {
        return vec;
    }
    if (!src->data || !vec->data) {
        genVec_destroy(vec);
        printf("copy: ivalid data pointers\n");
        return NULL;
    }

    memcpy(vec->data, src->data, src->size * src->data_size);
    vec->size = src->size;

    return vec;
}

void genVec_print(const genVec* vec, genVec_print_fn fn) { 
    if (!vec) {
        printf("print: vec is null\n");
        return;
    }
    if (!fn) {
        printf("print: print func is null\n");
        return; 
    }

    printf("[ ");
    for (size_t i = 0; i < vec->size; i++) {
        u8* element = vec->data + (i * vec->data_size);
        fn(element); 
        printf(" ");
    }
    printf("]\n");

    printf("Size: %lu\n", vec->size);
    printf("Capacity: %lu\n", vec->capacity);
}

void genVec_grow(genVec* vec) {
    if (!vec) {
        printf("grow: vec is null\n");
        return;
    }

    size_t new_cap;
    if (vec->capacity < 4) { new_cap = vec->capacity + 1; }
    else {
        new_cap = (size_t)((double)vec->capacity * GROWTH); 
    }

    u8* new_data = realloc(vec->data, new_cap * vec->data_size);
    if (!new_data) { 
        printf("grow: realloc failed\n");
        return;
    }

    vec->data = new_data;
    vec->capacity = new_cap;
}


void genVec_shrink(genVec* vec) {
    if (!vec) {
        printf("shrink: vec is null\n ");
        return;
    }

    size_t reduced_cap = (size_t)((double)vec->capacity * SHRINK_BY);
    if (reduced_cap < vec->size || reduced_cap == 0) { return; }

    u8* new_data = realloc(vec->data, reduced_cap * vec->data_size);
    if (!new_data) {
        printf("shrink: realloc failed\n");
        return;
    }

    vec->data = new_data;
    vec->capacity = reduced_cap;
}


