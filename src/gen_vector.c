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

genVec* genVec_init_val(size_t n, const void* val, size_t data_size, genVec_delete_fn del_fn) {
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
        memcpy((char*)vec->data + (i * data_size), val,data_size);
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
            void* element = (char*)vec->data + (i * vec->data_size);
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
        void* element = (char*)vec->data + (i * vec->data_size);
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
    
    void* new_data = realloc(vec->data, new_capacity * vec->data_size);
    if (!new_data) {
        printf("reserve: realloc failed\n");
        return;
    }
    
    vec->data = new_data;
    vec->capacity = new_capacity;
}

void genVec_push(genVec* vec, const void* data) 
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

    char* next_to_last = (char*)vec->data + (vec->size * vec->data_size); 
    memcpy(next_to_last, data, vec->data_size);
    vec->size++;
}

int genVec_pop(genVec* vec, void* popped) {
    if (!vec) {
        printf("pop: vec is null\n");
        return -1;
    }
    if (vec->size == 0) {
        printf("pop: vec is empty\n");
        return -1;
    }
    
    char* last_elm = (char*)vec->data + ((vec->size - 1) * vec->data_size);
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

void genVec_get(genVec* vec, size_t i, void* out) {
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

    char* elm = (char*)vec->data + (i * vec->data_size);
    memcpy(out, elm, vec->data_size);
}


void genVec_replace(genVec* vec, size_t i, const void* data) {
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

    char* to_replace = (char*)vec->data + (i * vec->data_size); 

    if (vec->del_fn) {
        vec->del_fn(to_replace);
    }

    memcpy(to_replace, data, vec->data_size);
}

void genVec_front(genVec* vec, void* out) {
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


void genVec_back(genVec* vec, void* out) {
    if (!vec) {
        printf("front: vec is null\n");
        return;
    }
    if (vec->size == 0) {
        printf("front: vec is empty\n");
        return;
    }
    
    char* last_elm = (char*)vec->data + ((vec->size - 1) * vec->data_size);
    memcpy(out, last_elm, vec->data_size);
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
        void* element = (char*)vec->data + (i * vec->data_size);
        vec->del_fn(element);
    }
        // Calculate the number of elements to shift
    size_t elements_to_shift = vec->size - i - 1;
    
    if (elements_to_shift > 0) {
        // Shift elements left to overwrite the deleted element
        char* dest = (char*)vec->data + (i * vec->data_size);
        char* src = (char*)vec->data + ((i + 1) * vec->data_size);
        
        memmove(dest, src, elements_to_shift * vec->data_size);  // Use memmove for overlapping regions
    }

    vec->size--;
    
    if (vec->size <= (size_t)((double)vec->capacity * SHRINK_AT)) 
        { genVec_shrink(vec); }
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

    memcpy(vec->data, src->data, src->size * src->data_size);
    vec->size = src->size;

    return vec;
}

void genVec_print(genVec* vec, genVec_print_fn fn) { 
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
        void* element = (char*)vec->data + (i * vec->data_size);
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
    if (vec->capacity == 0) { new_cap = 1; }
    else if (vec->capacity < 4) { new_cap = vec->capacity + 1; }
    else {
        new_cap = (size_t)((double)vec->capacity * GROWTH); 
    }

    void* new_data = realloc(vec->data, new_cap * vec->data_size);
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

    void* new_data = realloc(vec->data, reduced_cap * vec->data_size);
    if (!new_data) {
        printf("shrink: realloc failed\n");
        return;
    }

    vec->data = new_data;
    vec->capacity = reduced_cap;
}


