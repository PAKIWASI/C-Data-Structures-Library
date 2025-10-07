#pragma once

#include <stddef.h>
#include <stdint.h>


typedef uint8_t u8;

//user provided
typedef int (*genVec_compare_fn)(const u8* a, const u8* b);
typedef void (*genVec_print_fn)(const u8* elm);
typedef void (*genVec_delete_fn)(const u8* elm);


typedef struct {
    u8* data;
    size_t size;
    size_t capacity;
    size_t data_size;
    genVec_delete_fn del_fn;
} __attribute__((aligned(64))) genVec;


//memory management
genVec* genVec_init(size_t n, size_t data_size, genVec_delete_fn del_fn);
genVec* genVec_init_val(size_t n, const u8* val, size_t data_size, genVec_delete_fn del_fn);
void genVec_destroy(genVec* vec);
void genVec_clear(genVec* vec);
void genVec_reserve(genVec* vec, size_t new_capacity);
//operations
void genVec_push(genVec* vec, const u8* data);
int genVec_pop(genVec* vec, u8* popped);
void genVec_get(genVec* vec, size_t i, u8* out);
void genVec_replace(genVec* vec, size_t i, const u8* data);
void genVec_front(genVec* vec, u8* out);
void genVec_back(genVec* vec, u8* out);
void genVec_remove(genVec* vec, size_t i);
//TODO: iterators

//utility
genVec* genVec_copy(genVec* src);
void genVec_print(genVec* vec, genVec_print_fn fn);

size_t genVec_size(genVec* vec) { return vec->size; }


