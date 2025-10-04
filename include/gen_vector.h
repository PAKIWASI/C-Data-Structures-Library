#pragma once
#include <stddef.h>



//user provided
typedef int (*genVec_compare_fn)(const void* a, const void* b);
typedef void (*genVec_print_fn)(const void* elm);
typedef void (*genVec_delete_fn)(const void* elm);


typedef struct {
    void* data;
    size_t size;
    size_t capacity;
    size_t data_size;
    genVec_delete_fn del_fn;
} __attribute__((aligned(64))) genVec;


//memory management
genVec* genVec_init(size_t n, size_t data_size, genVec_delete_fn del_fn);
genVec* genVec_init_val(size_t n, const void* val, size_t data_size, genVec_delete_fn del_fn);
void genVec_destroy(genVec* vec);
void genVec_clear(genVec* vec);
void genVec_reserve(genVec* vec, size_t new_capacity);
//operations
void genVec_push(genVec* vec, const void* data);
int genVec_pop(genVec* vec, void* popped);
void genVec_get(genVec* vec, size_t i, void* out);
void genVec_replace(genVec* vec, size_t i, const void* data);
void genVec_front(genVec* vec, void* out);
void genVec_back(genVec* vec, void* out);
void genVec_remove(genVec* vec, size_t i);
//TODO: iterators

//utility
genVec* genVec_copy(genVec* src);
void genVec_print(genVec* vec, genVec_print_fn fn);

