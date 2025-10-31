#pragma once


#include "gen_vector.h"
typedef struct {
    genVec* arr;
} bitVec;


bitVec* bitVec_create(void);
void bitVec_destroy(bitVec* bvec);
