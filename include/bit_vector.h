#ifndef BIT_VECTOR_H
#define BIT_VECTOR_H

#include "gen_vector.h"


typedef struct {
    genVec* arr;
    u32 size;        // no of set bits
} bitVec;



bitVec* bitVec_create(void);
void bitVec_destroy(bitVec* bvec);


void bitVec_set(bitVec* bvec, u32 i);
void bitVec_clear(bitVec* bvec, u32 i);
u8 bitVec_test(bitVec* bvec, u32 i);
void bitVec_toggle(bitVec* bvec, u32 i);


void bitVec_push(bitVec* bvec);
void bitVec_pop(bitVec* bvec);


void bitVec_print(bitVec* bvec, u32 byteI);


// get the no of BITS in the vector
static inline u32 bitVec_size_bits(bitVec* bvec) {
    return bvec->size;
}

// get the no of BYTES in the vector
static inline u32 bitVec_size_bytes(bitVec* bvec) {
    return genVec_size(bvec->arr);
}


#endif // BIT_VECTOR_H
