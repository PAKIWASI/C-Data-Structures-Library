#ifndef MATRIX_H
#define MATRIX_H

#include "common.h"


// ROW MAJOR 2D MATRIX

#define MATRIX_TOTAL(mat)    ((u32)((mat)->n * (mat)->m))
#define IDX(mat, i, j)       (((i) * (mat)->n) + (j))
#define MATRIX_AT(mat, i, j) ((mat)->data[((i) * (mat)->n) + (j)])

#define ZEROS_1D(n)    ((float[n]){0})
#define ZEROS_2D(m, n) ((float[m][n]){0})


typedef struct {
    float* data;
    u32  m; // rows
    u32  n; // cols
} Matrix;


// CREATION AND DESTRUCTION
// ============================================================================

// create heap matrix with m rows and n cols
Matrix* matrix_create(u32 m, u32 n);

// create heap matrix with m rows and n cols and an array of size m x n
Matrix* matrix_create_arr(u32 m, u32 n, const float* arr);

// create matrix with everything on the stack
void matrix_create_stk(Matrix* mat, u32 m, u32 n, float* data);

// destroy the matrix created with matrix_create or matrix_create_arr
// DO NOT use on stack-allocated matrices (created with matrix_create_stk)
void matrix_destroy(Matrix* mat);


// SETTERS
// ============================================================================

/* Preferred method for setting values from arrays
   For direct arrays (float[len]){...} ROW MAJOR or (float[row][col]){{...},{...}} MORE EXPLICIT
   
   Usage:
       matrix_set_val_arr(mat, 9, (float*)(float[3][3]){
           {1, 2, 3},
           {4, 5, 6},
           {7, 8, 9}
       });
*/
void matrix_set_val_arr(Matrix* mat, u32 count, const float* arr);

// for 2D arrays (array of pointers)
void matrix_set_val_arr2(Matrix* mat, u32 m, u32 n, const float** arr2);

// set the value at position (i, j) where i is row and j is column
void matrix_set_elm(Matrix* mat, float elm, u32 i, u32 j);


// BASIC OPERATIONS
// ============================================================================

// Matrix addition: out = a + b
// out may alias a and/or b (safe to do: matrix_add(a, a, b))
void matrix_add(Matrix* out, const Matrix* a, const Matrix* b);

// Matrix subtraction: out = a - b
// out may alias a and/or b (safe to do: matrix_sub(a, a, b))
void matrix_sub(Matrix* out, const Matrix* a, const Matrix* b);

// Scalar multiplication: mat = mat * val
void matrix_scale(Matrix* mat, float val);

// Element wise divistion
void matrix_div(Matrix* mat, float val);

// Matrix copy: dest = src
void matrix_copy(Matrix* dest, const Matrix* src);


// MATRIX MULTIPLICATION
// ============================================================================

// Matrix multiplication: out = a × b
// (m×k) * (k×n) = (m×n)
// out may NOT alias a or b
// Uses blocked ikj multiplication for cache efficiency (good for small-medium matrices)
void matrix_xply(Matrix* out, const Matrix* a, const Matrix* b);

// Matrix multiplication variant 2: out = a × b
// Transposes b internally for better cache locality
// Takes more memory but can be faster for large matrices
// out may NOT alias a or b
void matrix_xply_2(Matrix* out, const Matrix* a, const Matrix* b);


// ADVANCED OPERATIONS
// ============================================================================

// Transpose: out = mat^T
// out may NOT alias mat
void matrix_T(Matrix* out, const Matrix* mat);

// LU Decomposition: mat = L × U
// Decomposes square matrix into Lower and Upper triangular matrices
void matrix_LU_Decomp(Matrix* L, Matrix* U, const Matrix* mat);

// Calculate determinant using LU decomposition
float matrix_det(const Matrix* mat);

// Calculate adjugate (adjoint) matrix
// TODO: NOT IMPLEMENTED - placeholder for future implementation
void matrix_adj(Matrix* out, const Matrix* mat);

// Calculate matrix inverse: out = mat^(-1)
// TODO: NOT IMPLEMENTED - placeholder for future implementation
void matrix_inv(Matrix* out, const Matrix* mat);


// UTILITIES
// ============================================================================

// print the formatted, aligned matrix to stdout
void matrix_print(const Matrix* mat);


/* 
 FUTURE ENHANCEMENTS / TODO:
 
 1. Generic macro approach (see matrix_generic.h for implementation)
    - Support for multiple data types (int, float, double, etc.)
    - Compile-time type safety
    
 2. SIMD optimization
    - Vectorized operations for add, sub, multiply
    - Platform-specific optimizations (SSE, AVX, NEON)
    
 3. Matrix views (zero-copy slicing)
    typedef struct {
        int* data;
        u32  n, m;
        u32  stride;    // for non-contiguous access
        u32  offset;    // starting position
    } MatrixView;
    
    - Slicing without copying
    - Transpose views without data movement
    - SIMD-friendly kernels with custom strides
    
 4. Identity matrix creation
    Matrix* matrix_iden(u32 n, u32 m);
    
 5. Additional operations
    - Matrix division (element-wise)
    - Power operations
    - Trace
    - Rank calculation
    - Eigenvalues/eigenvectors
    
 6. Numerical stability improvements
    - Pivoting for LU decomposition
    - Condition number checks
    - Iterative refinement
*/


#endif // MATRIX_H
