#ifndef MATRIX_H
#define MATRIX_H

#include "common.h"

typedef void (*matrix_print_fn)(const u8* elm);
#define MATRIX_TOTAL(mat) ((u32)((mat)->n * (mat)->m))


// ROW MAJOR 2D MATRIX

// we only need this for basic types !
// should support and n x m matrix (not dynamic)
// constant size once created


typedef struct {
    u8* data;
    u32 n;      // rows
    u32 m;      // cols
} Matrix;



Matrix* matrix_create(u32 n, u32 m);
void    matrix_create_stk(Matrix* mat, u8* data, u32 n, u32 m);
void    matrix_destroy(Matrix* mat);

void matrix_set_val(Matrix* mat, ...); // TODO: take input as rows?

void matrix_add(Matrix* out, Matrix* a, Matrix* b);
void matrix_xply(Matrix* out, Matrix* a, Matrix* b);

void matrix_add_self(Matrix* a, Matrix* b);
void matrix_xply_self(Matrix* a, Matrix* b);

void matrix_T(Matrix* out, Matrix mat);
void matrix_T_self(Matrix* mat);

void matrix_scale(Matrix* mat, u8* val);


void matrix_print(Matrix* mat, matrix_print_fn print_fn);

#endif // MATRIX_H
