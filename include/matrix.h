#ifndef MATRIX_H
#define MATRIX_H

#include "common.h"


// TODO: macro approach

// ROW MAJOR 2D MATRIX

#define MATRIX_TOTAL(mat) ((u32)((mat)->n * (mat)->m))




typedef struct {
    int* data;
    u32  n; // rows
    u32  m; // cols
} Matrix;



// create heap matrix with n rows and m cols
Matrix* matrix_create(u32 n, u32 m);
// create heap matrix with n rows and m cols and an array of size n x m
Matrix* matrix_create_arr(u32 n, u32 m, const int* arr);
// create matrix with everything on the stack
void    matrix_create_stk(Matrix* mat, int* data, u32 n, u32 m);
// destroy the matrix created with matrix_create and matrix_create_stk
void    matrix_destroy(Matrix* mat);


// varadic function (no bounds checking or count check)
void matrix_set_val(Matrix* mat, ...); 

/* for direct arrays ( (int[len]){....} ROW MAJOR) or ( (int[row][col]){{...},{...},,} MORE EXPLICIT)
Usage:
    matrix_set_val_arr(mat, (int*)(int[3][3]){
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 8}
    }, 9);
*/
void matrix_set_val_arr(Matrix* mat, const int* arr, u32 count); 

// for 2D arrays (points of pointers)
void matrix_set_val_arr2(Matrix* mat, const int** arr2, u32 n, u32 m);

// set the value at position i, j
void matrix_set_elm(Matrix* mat, int elm, u32 i, u32 j);

void matrix_add(Matrix* out, Matrix* a, Matrix* b);
void matrix_xply(Matrix* out, Matrix* a, Matrix* b);

void matrix_add_self(Matrix* a, Matrix* b);
void matrix_xply_self(Matrix* a, Matrix* b);

void matrix_T(Matrix* out, Matrix* mat);
void matrix_T_self(Matrix* mat);

void matrix_scale(Matrix* mat, int* val);

void matrix_copy(Matrix* out, Matrix* mat);

// print the formatted, alighmed matrix
void matrix_print(Matrix* mat);

#endif // MATRIX_H
