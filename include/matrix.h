#ifndef MATRIX_H
#define MATRIX_H

#include "common.h"


// we only need this for basic types !
// should support and n x m matrix (not dynamic)
// constant size once created


typedef struct {
    u8* data;
    int n;      // rows
    int m;      // cols
} Matrix;



Matrix* matrix_create(int n, int m);
Matrix* matrix_create_stk(Matrix* matrix, int n, int m);



#endif // MATRIX_H
