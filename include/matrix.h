#ifndef MATRIX_H
#define MATRIX_H

#include "common.h"


// TODO: 
// 1. macro approach
// 2. SIMD?

// ROW MAJOR 2D MATRIX

#define MATRIX_TOTAL(mat) ((u32)((mat)->n * (mat)->m))
#define ZEROS_1D(n) ((int[n]{0}))
#define ZEROS_2D(n, m) ((int[n][m]){0})



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
void    matrix_create_stk(Matrix* mat, u32 n, u32 m, int* data);

// create an identity matrix of dimenstions n x m
//Matrix* matrix_iden(u32 n, u32 m);

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

// add a and b and save result in out
// out may alias a and/or b
void matrix_add(Matrix* out, const Matrix* a, const Matrix* b);

void matrix_sub(Matrix* out, const Matrix* a, const Matrix* b);

// out may not alias a, b (TODO: will need _self functions when data shape changes)
void matrix_xply(Matrix* out, const Matrix* a, const Matrix* b);
void matrix_xply_self(Matrix* a, const Matrix* b);

void matrix_div(Matrix* out, const Matrix* a, const Matrix* b);
void matrix_div_self(Matrix* a, const Matrix* b);

void matrix_T(Matrix* out, const Matrix* mat);
void matrix_T_self(Matrix* mat);

int  matrix_det(const Matrix* mat);

void matrix_adj(Matrix* out, const Matrix* mat);
//void matrix_adj_self(Matrix* mat);

void matrix_inv(Matrix* out, const Matrix* mat);
//void matrix_inv_self(Matrix* mat);

void matrix_scale(Matrix* mat, int val);


void matrix_copy(Matrix* dest, const Matrix* src);

// print the formatted, alighmed matrix
void matrix_print(const Matrix* mat);

/* TODO: 
 
typedef struct {
    int* data;
    u32  n, m;
    u32  stride;
} MatrixView;

slicing,transposes without copying,SIMD-friendly kernels

*/

#endif // MATRIX_H
