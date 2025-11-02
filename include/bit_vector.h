#pragma once

#include "gen_vector.h"
#include <stddef.h>


typedef struct {
    genVec* arr;
    size_t size;
} __attribute__((aligned(16))) bitVec;



bitVec* bitVec_create(void);
void bitVec_destroy(bitVec* bvec);


void bitVec_set(bitVec* bvec, size_t i);
void bitVec_clear(bitVec* bvec, size_t i);
u8 bitVec_test(bitVec* bvec, size_t i);
void bitVec_toggle(bitVec* bvec, size_t i);


void bitVec_push(bitVec* bvec);
void bitVec_pop(bitVec* bvec);


void bitVec_print(bitVec* bvec, size_t byteI);


// get the no of BITS in the vector
static inline size_t bitVec_size_bits(bitVec* bvec) {
    return bvec->size;
}

static inline size_t bitVec_size_bytes(bitVec* bvec) {
    return bvec->arr->size;
}
