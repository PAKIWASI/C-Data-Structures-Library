#ifndef MATRIX_H
#define MATRIX_H

#include "common.h"


// ROW MAJOR 2D MATRIX

#define MATRIX_TOTAL(mat)    ((u32)((mat)->n * (mat)->m))
#define IDX(mat, i, j)       (((i) * (mat)->n) + (j))
#define MATRIX_AT(mat, i, j) ((mat)->data[((i) * (mat)->n) + (j)])

#define ZEROS_1D(n)    ((int[n]){0})
#define ZEROS_2D(m, n) ((int[m][n]){0})


typedef struct {
    int* data;
    u32  m; // rows
    u32  n; // cols
} Matrix;

// create heap matrix with n rows and m cols
Matrix* matrix_create(u32 m, u32 n);

// create heap matrix with n rows and m cols and an array of size n x m
Matrix* matrix_create_arr(u32 m, u32 n, const int* arr);

// create matrix with everything on the stack
void matrix_create_stk(Matrix* mat, u32 m, u32 n, int* data);

// create an identity matrix of dimenstions n x m
//Matrix* matrix_iden(u32 n, u32 m);

// destroy the matrix created with matrix_create and matrix_create_stk
void matrix_destroy(Matrix* mat);


// varadic function (no bounds checking or count check)
void matrix_set_val(Matrix* mat, ...);

/* Preferred method: 
for direct arrays ( (int[len]){....} ROW MAJOR) or ( (int[row][col]){{...},{...},,} MORE EXPLICIT)
Usage:
    matrix_set_val_arr(mat, (int*)(int[3][3]){
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 8}
    }, 9);
*/
void matrix_set_val_arr(Matrix* mat, u32 count, const int* arr);

// for 2D arrays (points of pointers)
void matrix_set_val_arr2(Matrix* mat, u32 m, u32 n, const int** arr2);

// set the value at position i, j
void matrix_set_elm(Matrix* mat, int elm, u32 i, u32 j);

// add a and b and save result in out
// out may alias a and/or b
void matrix_add(Matrix* out, const Matrix* a, const Matrix* b);

// out may alias a and/or b
void matrix_sub(Matrix* out, const Matrix* a, const Matrix* b);

// out may not alias a, b (TODO: will need _self functions when data shape changes)
void matrix_xply(Matrix* out, const Matrix* a, const Matrix* b);
void matrix_xply_2(Matrix* out, const Matrix* a, const Matrix* b);

// void matrix_div(Matrix* out, const Matrix* a, const Matrix* b);

void matrix_T(Matrix* out, const Matrix* mat);

void matrix_LU_Decomp(Matrix* L, Matrix* U, const Matrix* mat);

int matrix_det(const Matrix* mat);

void matrix_adj(Matrix* out, const Matrix* mat);

void matrix_inv(Matrix* out, const Matrix* mat);

void matrix_scale(Matrix* mat, int val);

void matrix_copy(Matrix* dest, const Matrix* src);

// print the formatted, alighmed matrix
void matrix_print(const Matrix* mat);

/* 
 TODO: 
 1. macro approach
 2. SIMD?

typedef struct {
    int* data;
    u32  n, m;
    u32  stride;
} MatrixView;

slicing,transposes without copying,SIMD-friendly kernels

*/



#include <stdarg.h>

// ============================================================================
// CORE MACROS (used by both declaration and implementation)
// ============================================================================

#define MATRIX_STRUCT_NAME(type)  Matrix_##type
#define MATRIX_CREATE_NAME(type)  matrix_create_##type
#define MATRIX_DESTROY_NAME(type) matrix_destroy_##type
#define MATRIX_PRINT_NAME(type)   matrix_print_##type
// ... add more as needed

// Helper macros for array definitions
#define ZEROS_1D_T(type, n)    ((type[n]){0})
#define ZEROS_2D_T(type, m, n) ((type[m][n]){0})

// ============================================================================
// DECLARATION MACROS
// ============================================================================

// Declare a single matrix type
#define DECLARE_MATRIX_TYPE(type) \
    typedef struct {              \
        type* data;               \
        u32   m;                  \
        u32   n;                  \
    } Matrix_##type;

// Declare all functions for a type
#define DECLARE_MATRIX_FUNCTIONS(type)                                        \
    /* Creation/ Destruction */                                               \
    Matrix_##type* matrix_create_##type(u32 m, u32 n);                        \
    Matrix_##type* matrix_create_arr_##type(u32 m, u32 n, const type* arr);   \
    void           matrix_create_stk_##type(Matrix_##type* mat, u32 m, u32 n, \
                                            type* data);                      \
    void           matrix_destroy_##type(Matrix_##type* mat);                 \
                                                                              \
    /* Setting values */                                                      \
    void matrix_set_val_##type(Matrix_##type* mat, ...);                      \
    void matrix_set_val_arr_##type(Matrix_##type* mat, u32 count,             \
                                   const type* arr);                          \
    void matrix_set_val_arr2_##type(Matrix_##type* mat, u32 m, u32 n,         \
                                    const type** arr2);                       \
    void matrix_set_elm_##type(Matrix_##type* mat, type elm, u32 i, u32 j);   \
                                                                              \
    /* Operations */                                                          \
    void matrix_add_##type(Matrix_##type* out, const Matrix_##type* a,        \
                           const Matrix_##type* b);                           \
    void matrix_sub_##type(Matrix_##type* out, const Matrix_##type* a,        \
                           const Matrix_##type* b);                           \
    void matrix_xply_##type(Matrix_##type* out, const Matrix_##type* a,       \
                            const Matrix_##type* b);                          \
    void matrix_xply_2_##type(Matrix_##type* out, const Matrix_##type* a,     \
                              const Matrix_##type* b);                        \
    void matrix_T_##type(Matrix_##type* out, const Matrix_##type* mat);       \
                                                                              \
    /* Advanced operations */                                                 \
    void matrix_LU_Decomp_##type(Matrix_##type* L, Matrix_##type* U,          \
                                 const Matrix_##type* mat);                   \
    type matrix_det_##type(const Matrix_##type* mat);                         \
    void matrix_scale_##type(Matrix_##type* mat, type val);                   \
    void matrix_copy_##type(Matrix_##type* dest, const Matrix_##type* src);   \
                                                                              \
    /* Utility */                                                             \
    void matrix_print_##type(const Matrix_##type* mat);

// Combined declaration macro
#define DECLARE_MATRIX_FULL(type) \
    DECLARE_MATRIX_TYPE(type)     \
    DECLARE_MATRIX_FUNCTIONS(type)

// ============================================================================
// X-MACRO DISPATCHER FOR DECLARATIONS
// ============================================================================

// First pass: declare all types
#define X(type, suffix, fmt) DECLARE_MATRIX_TYPE(type);
#include "matrix_types.def"
#undef X

// Second pass: declare all functions
#define X(type, suffix, fmt) DECLARE_MATRIX_FUNCTIONS(type);
#include "matrix_types.def"
#undef X



// #ifdef MATRIX_IMPLEMENTATION
// #define MATRIX_IMPLEMENT_FUNCTIONS(type)                                       \
//                                                                                \
//     Matrix_##type* matrix_create_##type(u32 m, u32 n)                          \
//     {                                                                          \
//         CHECK_FATAL(n == 0 && m == 0, "n == m == 0");                          \
//         Matrix_##type* mat = (Matrix_##type*)malloc(sizeof(Matrix_##type));    \
//         CHECK_FATAL(!mat, "matrix malloc failed");                             \
//         mat->m    = m;                                                         \
//         mat->n    = n;                                                         \
//         mat->data = (type*)malloc(sizeof(type) * n * m);                       \
//         CHECK_FATAL(!mat->data, "matrix data malloc failed");                  \
//         return mat;                                                            \
//     }                                                                          \
//                                                                                \
//     Matrix_##type* matrix_create_arr_##type(u32 m, u32 n, const type* arr)     \
//     {                                                                          \
//         CHECK_FATAL(!arr, "input arr is null");                                \
//         Matrix_##type* mat = matrix_create_##type(m, n);                       \
//         memcpy(mat->data, arr, sizeof(type) * m * n);                          \
//         return mat;                                                            \
//     }                                                                          \
//                                                                                \
//     void matrix_create_stk_##type(Matrix_##type* mat, u32 m, u32 n,            \
//                                   type* data)                                  \
//     {                                                                          \
//         CHECK_FATAL(!mat, "matrix is null");                                   \
//         CHECK_FATAL(!data, "data is null");                                    \
//         mat->data = data;                                                      \
//         mat->m    = m;                                                         \
//         mat->n    = n;                                                         \
//     }                                                                          \
//                                                                                \
//     void matrix_destroy_##type(Matrix_##type* mat)                             \
//     {                                                                          \
//         CHECK_FATAL(!mat, "matrix is null");                                   \
//         free(mat->data);                                                       \
//         free(mat);                                                             \
//     }                                                                          \
//                                                                                \
//     void matrix_set_val_##type(Matrix_##type* mat, ...)                        \
//     {                                                                          \
//         CHECK_FATAL(!mat, "matrix is null");                                   \
//         va_list arr;                                                           \
//         va_start(arr, mat);                                                    \
//         u32 total = MATRIX_TOTAL(mat);                                         \
//         for (u32 i = 0; i < total; i++) {                                      \
//             mat->data[i] = (type)va_arg(arr, double);                          \
//         }                                                                      \
//         va_end(arr);                                                           \
//     }                                                                          \
//                                                                                \
//     void matrix_set_val_arr_##type(Matrix_##type* mat, u32 count,              \
//                                    const type* arr)                            \
//     {                                                                          \
//         CHECK_FATAL(!mat, "matrix is null");                                   \
//         CHECK_FATAL(!arr, "arr is null");                                      \
//         CHECK_FATAL(count != MATRIX_TOTAL(mat), "count doesn't match size");   \
//         memcpy(mat->data, arr, sizeof(type) * count);                          \
//     }                                                                          \
//                                                                                \
//     void matrix_set_val_arr2_##type(Matrix_##type* mat, u32 m, u32 n,          \
//                                     const type** arr2)                         \
//     {                                                                          \
//         CHECK_FATAL(!mat, "matrix is null");                                   \
//         CHECK_FATAL(!arr2, "arr is null");                                     \
//         CHECK_FATAL(!*arr2, "*arr is null");                                   \
//         CHECK_FATAL(m != mat->m || n != mat->n, "dimensions don't match");     \
//         u32 idx = 0;                                                           \
//         for (u32 i = 0; i < m; i++) {                                          \
//             memcpy(mat->data + idx, arr2[i], sizeof(type) * n);                \
//             idx += n;                                                          \
//         }                                                                      \
//     }                                                                          \
//                                                                                \
//     void matrix_set_elm_##type(Matrix_##type* mat, type elm, u32 i, u32 j)     \
//     {                                                                          \
//         CHECK_FATAL(!mat, "matrix is null");                                   \
//         CHECK_FATAL(i >= mat->m || j >= mat->n, "index out of bounds");        \
//         mat->data[IDX(mat, i, j)] = elm;                                       \
//     }                                                                          \
//                                                                                \
//     void matrix_add_##type(Matrix_##type* out, const Matrix_##type* a,         \
//                            const Matrix_##type* b)                             \
//     {                                                                          \
//         CHECK_FATAL(!out, "out matrix is null");                               \
//         CHECK_FATAL(!a, "a matrix is null");                                   \
//         CHECK_FATAL(!b, "b matrix is null");                                   \
//         CHECK_FATAL(a->m != b->m || a->n != b->n || a->m != out->m ||          \
//                         a->n != out->n,                                        \
//                     "dimensions don't match");                                 \
//         u32 total = MATRIX_TOTAL(a);                                           \
//         for (u32 i = 0; i < total; i++) {                                      \
//             out->data[i] = a->data[i] + b->data[i];                            \
//         }                                                                      \
//     }                                                                          \
//                                                                                \
//     void matrix_sub_##type(Matrix_##type* out, const Matrix_##type* a,         \
//                            const Matrix_##type* b)                             \
//     {                                                                          \
//         CHECK_FATAL(!out, "out matrix is null");                               \
//         CHECK_FATAL(!a, "a matrix is null");                                   \
//         CHECK_FATAL(!b, "b matrix is null");                                   \
//         CHECK_FATAL(a->m != b->m || a->n != b->n, "dimensions don't match");   \
//         u32 total = MATRIX_TOTAL(a);                                           \
//         for (u32 i = 0; i < total; i++) {                                      \
//             out->data[i] = a->data[i] - b->data[i];                            \
//         }                                                                      \
//     }                                                                          \
//                                                                                \
//     void matrix_xply_##type(Matrix_##type* out, const Matrix_##type* a,        \
//                             const Matrix_##type* b)                            \
//     {                                                                          \
//         CHECK_FATAL(!out, "out matrix is null");                               \
//         CHECK_FATAL(!a, "a matrix is null");                                   \
//         CHECK_FATAL(!b, "b matrix is null");                                   \
//         CHECK_FATAL(a->n != b->m, "incompatible dimensions");                  \
//         CHECK_FATAL(out->m != a->m || out->n != b->n,                          \
//                     "output has wrong dimensions");                            \
//         u32 m = a->m;                                                          \
//         u32 k = a->n;                                                          \
//         u32 n = b->n;                                                          \
//         memset(out->data, 0, sizeof(type) * m * n);                            \
//         const u32 BLOCK_SIZE = 16;                                             \
//         for (u32 i = 0; i < m; i += BLOCK_SIZE) {                              \
//             for (u32 k_outer = 0; k_outer < k; k_outer += BLOCK_SIZE) {        \
//                 for (u32 j = 0; j < n; j += BLOCK_SIZE) {                      \
//                     u32 i_max = (i + BLOCK_SIZE < m) ? i + BLOCK_SIZE : m;     \
//                     u32 k_max =                                                \
//                         (k_outer + BLOCK_SIZE < k) ? k_outer + BLOCK_SIZE : k; \
//                     u32 j_max = (j + BLOCK_SIZE < n) ? j + BLOCK_SIZE : n;     \
//                     for (u32 ii = i; ii < i_max; ii++) {                       \
//                         for (u32 kk = k_outer; kk < k_max; kk++) {             \
//                             type a_val = a->data[IDX(a, ii, kk)];              \
//                             for (u32 jj = j; jj < j_max; jj++) {               \
//                                 out->data[IDX(out, ii, jj)] +=                 \
//                                     a_val * b->data[IDX(b, kk, jj)];           \
//                             }                                                  \
//                         }                                                      \
//                     }                                                          \
//                 }                                                              \
//             }                                                                  \
//         }                                                                      \
//     }                                                                          \
//                                                                                \
//     void matrix_xply_2_##type(Matrix_##type* out, const Matrix_##type* a,      \
//                               const Matrix_##type* b)                          \
//     {                                                                          \
//         CHECK_FATAL(!out, "out matrix is null");                               \
//         CHECK_FATAL(!a, "a matrix is null");                                   \
//         CHECK_FATAL(!b, "b matrix is null");                                   \
//         CHECK_FATAL(a->n != b->m, "incompatible dimensions");                  \
//         CHECK_FATAL(out->m != a->m || out->n != b->n,                          \
//                     "output has wrong dimensions");                            \
//         u32            m   = a->m;                                             \
//         u32            k   = a->n;                                             \
//         u32            n   = b->n;                                             \
//         Matrix_##type* b_T = matrix_create_##type(n, k);                       \
//         matrix_T_##type(b_T, b);                                               \
//         memset(out->data, 0, sizeof(type) * m * n);                            \
//         const u32 BLOCK_SIZE = 16;                                             \
//         for (u32 i = 0; i < m; i += BLOCK_SIZE) {                              \
//             for (u32 j = 0; j < n; j += BLOCK_SIZE) {                          \
//                 u32 i_max = (i + BLOCK_SIZE < m) ? i + BLOCK_SIZE : m;         \
//                 u32 j_max = (j + BLOCK_SIZE < n) ? j + BLOCK_SIZE : n;         \
//                 for (u32 ii = i; ii < i_max; ii++) {                           \
//                     for (u32 jj = j; jj < j_max; jj++) {                       \
//                         type sum = 0;                                          \
//                         for (u32 kk = 0; kk < k; kk++) {                       \
//                             sum += a->data[IDX(a, ii, kk)] *                   \
//                                    b_T->data[IDX(b_T, jj, kk)];                \
//                         }                                                      \
//                         out->data[IDX(out, ii, jj)] = sum;                     \
//                     }                                                          \
//                 }                                                              \
//             }                                                                  \
//         }                                                                      \
//         matrix_destroy_##type(b_T);                                            \
//     }                                                                          \
//                                                                                \
//     void matrix_T_##type(Matrix_##type* out, const Matrix_##type* mat)         \
//     {                                                                          \
//         CHECK_FATAL(!mat, "mat matrix is null");                               \
//         CHECK_FATAL(!out, "out matrix is null");                               \
//         CHECK_FATAL(mat->m != out->n || mat->n != out->m,                      \
//                     "incompatible dimensions");                                \
//         const u32 BLOCK_SIZE = 16;                                             \
//         for (u32 i = 0; i < mat->m; i += BLOCK_SIZE) {                         \
//             for (u32 j = 0; j < mat->n; j += BLOCK_SIZE) {                     \
//                 u32 i_max =                                                    \
//                     (i + BLOCK_SIZE < mat->m) ? i + BLOCK_SIZE : mat->m;       \
//                 u32 j_max =                                                    \
//                     (j + BLOCK_SIZE < mat->n) ? j + BLOCK_SIZE : mat->n;       \
//                 for (u32 ii = i; ii < i_max; ii++) {                           \
//                     for (u32 jj = j; jj < j_max; jj++) {                       \
//                         out->data[IDX(out, jj, ii)] =                          \
//                             mat->data[IDX(mat, ii, jj)];                       \
//                     }                                                          \
//                 }                                                              \
//             }                                                                  \
//         }                                                                      \
//     }                                                                          \
//                                                                                \
//     void matrix_LU_Decomp_##type(Matrix_##type* L, Matrix_##type* U,           \
//                                  const Matrix_##type* mat)                     \
//     {                                                                          \
//         CHECK_FATAL(!L, "L mat is null");                                      \
//         CHECK_FATAL(!U, "U mat is null");                                      \
//         CHECK_FATAL(!mat, "mat is null");                                      \
//         CHECK_FATAL(mat->n != mat->m, "mat is not square");                    \
//         CHECK_FATAL(L->n != mat->n || L->m != mat->m,                          \
//                     "L dimensions don't match");                               \
//         CHECK_FATAL(U->n != mat->n || U->m != mat->m,                          \
//                     "U dimensions don't match");                               \
//         const u32 n = mat->n;                                                  \
//         memset(L->data, 0, sizeof(type) * n * n);                              \
//         memset(U->data, 0, sizeof(type) * n * n);                              \
//         for (u32 i = 0; i < n; i++) { L->data[IDX(L, i, i)] = 1; }             \
//         for (u32 i = 0; i < n; i++) {                                          \
//             for (u32 k = i; k < n; k++) {                                      \
//                 type sum = 0;                                                  \
//                 for (u32 j = 0; j < i; j++) {                                  \
//                     sum += L->data[IDX(L, i, j)] * U->data[IDX(U, j, k)];      \
//                 }                                                              \
//                 U->data[IDX(U, i, k)] = MATRIX_AT(mat, i, k) - sum;            \
//             }                                                                  \
//             for (u32 k = i + 1; k < n; k++) {                                  \
//                 type sum = 0;                                                  \
//                 for (u32 j = 0; j < i; j++) {                                  \
//                     sum += L->data[IDX(L, k, j)] * U->data[IDX(U, j, i)];      \
//                 }                                                              \
//                 CHECK_FATAL(U->data[IDX(U, i, i)] == 0,                        \
//                             "Matrix is singular - LU failed");                 \
//                 L->data[IDX(L, k, i)] =                                        \
//                     (MATRIX_AT(mat, k, i) - sum) / U->data[IDX(U, i, i)];      \
//             }                                                                  \
//         }                                                                      \
//     }                                                                          \
//                                                                                \
//     type matrix_det_##type(const Matrix_##type* mat)                           \
//     {                                                                          \
//         CHECK_FATAL(!mat, "mat matrix is null");                               \
//         CHECK_FATAL(mat->m != mat->n, "only square matrices have det");        \
//         const u32      n = mat->n;                                             \
//         Matrix_##type* L = matrix_create_##type(n, n);                         \
//         Matrix_##type* U = matrix_create_##type(n, n);                         \
//         matrix_LU_Decomp_##type(L, U, mat);                                    \
//         type det = 1;                                                          \
//         for (u32 i = 0; i < n; i++) { det *= U->data[IDX(U, i, i)]; }          \
//         matrix_destroy_##type(L);                                              \
//         matrix_destroy_##type(U);                                              \
//         return det;                                                            \
//     }                                                                          \
//                                                                                \
//     void matrix_scale_##type(Matrix_##type* mat, type val)                     \
//     {                                                                          \
//         CHECK_FATAL(!mat, "matrix is null");                                   \
//         u32 total = MATRIX_TOTAL(mat);                                         \
//         for (u32 i = 0; i < total; i++) { mat->data[i] *= val; }               \
//     }                                                                          \
//                                                                                \
//     void matrix_copy_##type(Matrix_##type* dest, const Matrix_##type* src)     \
//     {                                                                          \
//         CHECK_FATAL(!dest, "dest matrix is null");                             \
//         CHECK_FATAL(!src, "src matrix is null");                               \
//         CHECK_FATAL(dest->m != src->m || dest->n != src->n,                    \
//                     "dimensions don't match");                                 \
//         memcpy(dest->data, src->data, sizeof(type) * MATRIX_TOTAL(src));       \
//     }                                                                          \
//                                                                                \
//     void matrix_print_##type(const Matrix_##type* mat, const char* fmt)        \
//     {                                                                          \
//         CHECK_FATAL(!mat, "matrix is null");                                   \
//         CHECK_FATAL(!fmt, "format string is null");                            \
//         for (u32 i = 0; i < mat->m; i++) {                                     \
//             printf("| ");                                                      \
//             for (u32 j = 0; j < mat->n; j++) {                                 \
//                 printf(fmt, mat->data[IDX(mat, i, j)]);                        \
//                 printf(" ");                                                   \
//             }                                                                  \
//             printf("|\n");                                                     \
//         }                                                                      \
//     }
// #endif // MATRIX_IMPLEMENTATION


#endif // MATRIX_H
