#include "matrix.h"

#include <string.h>
#include <limits.h>



Matrix* matrix_create(u32 m, u32 n)
{
    CHECK_FATAL(n == 0 && m == 0, "n == m == 0");

    Matrix* mat = (Matrix*)malloc(sizeof(Matrix));
    CHECK_FATAL(!mat, "matrix malloc failed");

    mat->m    = m;
    mat->n    = n;
    mat->data = (int*)malloc(sizeof(int) * n * m);
    CHECK_FATAL(!mat->data, "matrix data malloc failed");

    return mat;
}

Matrix* matrix_create_arr(u32 m, u32 n, const int* arr)
{
    CHECK_FATAL(!arr, "input arr is null");
    Matrix* mat = matrix_create(m, n);
    memcpy(mat->data, arr, sizeof(int) * m * n);
    return mat;
}

void matrix_create_stk(Matrix* mat, u32 m, u32 n, int* data)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!data, "data is null");

    mat->data = data; // we can do this on the stack
    mat->m    = m;
    mat->n    = n;
}

void matrix_destroy(Matrix* mat)
{
    CHECK_FATAL(!mat, "matrix is null");

    free(mat->data);
    free(mat);
}


void matrix_set_val(Matrix* mat, ...)
{
    CHECK_FATAL(!mat, "matrix is null");

    va_list arr;
    va_start(arr, mat);
    u32 total = MATRIX_TOTAL(mat);
    for (u32 i = 0; i < total; i++) { mat->data[i] = va_arg(arr, int); }
    va_end(arr);
}

void matrix_set_val_arr(Matrix* mat, u32 count, const int* arr)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(count != MATRIX_TOTAL(mat), "count doesn't match matrix size");

    memcpy(mat->data, arr, sizeof(int) * count);
}

void matrix_set_val_arr2(Matrix* mat, u32 m, u32 n, const int** arr2)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!arr2, "arr is null");
    CHECK_FATAL(!*arr2, "*arr is null");
    CHECK_FATAL(m != mat->m || n != mat->n,
                "mat dimentions dont match passed arr2");

    u32 idx = 0;
    for (u32 i = 0; i < m; i++) {
        memcpy(mat->data + idx, arr2[i], sizeof(int) * n);
        idx += n;
    }
}

void matrix_set_elm(Matrix* mat, int elm, u32 i, u32 j)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(i >= mat->m || j >= mat->n, "index out of bounds");

    mat->data[IDX(mat, i, j)] = elm;
}

void matrix_add(Matrix* out, const Matrix* a, const Matrix* b)
{
    CHECK_FATAL(!out, "out matrix is null");
    CHECK_FATAL(!a, "a matrix is null");
    CHECK_FATAL(!b, "b matrix is null");
    CHECK_FATAL(a->m != b->m || a->n != b->n || a->m != out->m ||
                    a->n != out->n,
                "a, b, out mat dimentions dont match");

    u32 total = MATRIX_TOTAL(a);

    for (u32 i = 0; i < total; i++) { out->data[i] = a->data[i] + b->data[i]; }
}


void matrix_sub(Matrix* out, const Matrix* a, const Matrix* b)
{
    CHECK_FATAL(!out, "out matrix is null");
    CHECK_FATAL(!a, "a matrix is null");
    CHECK_FATAL(!b, "b matrix is null");
    CHECK_FATAL(a->m != b->m || a->n != b->n, "a, b mat dimentions dont match");

    u32 total = MATRIX_TOTAL(a);

    for (u32 i = 0; i < total; i++) { out->data[i] = a->data[i] - b->data[i]; }
}

// ikj multiplication. (mxk) * (kxn) = (mxn)
// this is good for small to medium size matrices
void matrix_xply(Matrix* out, const Matrix* a, const Matrix* b)
{
    CHECK_FATAL(!out, "out matrix is null");
    CHECK_FATAL(!a, "a matrix is null");
    CHECK_FATAL(!b, "b matrix is null");
    CHECK_FATAL(a->n != b->m,
                "incompatible matrix dimensions for multiplication");
    CHECK_FATAL(out->m != a->m || out->n != b->n,
                "output matrix has wrong dimensions");

    u32 m = a->m; // rows of A
    u32 k = a->n; // cols of A = rows of B
    u32 n = b->n; // cols of B

    // Initialize output to zero
    memset(out->data, 0, sizeof(int) * m * n);

    // Block size for cache optimization
    const u32 BLOCK_SIZE = 16;

    // Blocked matrix multiplication (ikj order for cache efficiency)
    for (u32 i = 0; i < m; i += BLOCK_SIZE) {
        for (u32 k_outer = 0; k_outer < k; k_outer += BLOCK_SIZE) {
            for (u32 j = 0; j < n; j += BLOCK_SIZE) {
                // Block boundaries
                u32 i_max = (i + BLOCK_SIZE < m) ? i + BLOCK_SIZE : m;
                u32 k_max =
                    (k_outer + BLOCK_SIZE < k) ? k_outer + BLOCK_SIZE : k;
                u32 j_max = (j + BLOCK_SIZE < n) ? j + BLOCK_SIZE : n;
                // Multiply this block
                for (u32 ii = i; ii < i_max; ii++) {
                    for (u32 kk = k_outer; kk < k_max; kk++) {

                        int a_val = a->data[IDX(a, ii, kk)];
                        for (u32 jj = j; jj < j_max; jj++) {
                            out->data[IDX(out, ii, jj)] +=
                                a_val * b->data[IDX(b, kk, jj)];
                        }
                    }
                }
            }
        }
    }
}


// this function transposes b for cache-friendly access
// takes more memory, good for large size matrices
void matrix_xply_2(Matrix* out, const Matrix* a, const Matrix* b)
{
    CHECK_FATAL(!out, "out matrix is null");
    CHECK_FATAL(!a, "a matrix is null");
    CHECK_FATAL(!b, "b matrix is null");
    CHECK_FATAL(a->n != b->m, "incompatible matrix dimensions");
    CHECK_FATAL(out->m != a->m || out->n != b->n,
                "output matrix has wrong dimensions");

    u32 m = a->m;
    u32 k = a->n;
    u32 n = b->n;

    // Transpose B for cache-friendly access
    Matrix* b_T = matrix_create(n, k); // B^T is n×k
    matrix_T(b_T, b);

    memset(out->data, 0, sizeof(int) * m * n);

    const u32 BLOCK_SIZE = 16;

    // Now both A and B^T are accessed row-wise
    for (u32 i = 0; i < m; i += BLOCK_SIZE) {
        for (u32 j = 0; j < n; j += BLOCK_SIZE) {
            u32 i_max = (i + BLOCK_SIZE < m) ? i + BLOCK_SIZE : m;
            u32 j_max = (j + BLOCK_SIZE < n) ? j + BLOCK_SIZE : n;

            for (u32 ii = i; ii < i_max; ii++) {
                for (u32 jj = j; jj < j_max; jj++) {

                    int sum = 0;
                    // Dot product of A[ii] and B_T[jj] (both row-wise)
                    for (u32 kk = 0; kk < k; kk++) {
                        sum += a->data[IDX(a, ii, kk)] *
                               b_T->data[IDX(b_T, jj, kk)];
                    }
                    out->data[IDX(out, ii, jj)] = sum;
                }
            }
        }
    }
    matrix_destroy(b_T);
}

/*
Doolittle algorithm computes U's i-th row, then L's i-th column, alternating
For each element, you subtract the dot product of already-computed L and U values
*/
void matrix_LU_Decomp(Matrix* L, Matrix* U, const Matrix* mat)
{
    CHECK_FATAL(!L, "L mat is null");
    CHECK_FATAL(!U, "U mat is null");
    CHECK_FATAL(!mat, "mat is null");
    CHECK_FATAL(mat->n != mat->m, "mat is not a square matrix");
    CHECK_FATAL(L->n != mat->n || L->m != mat->m, "L dimensions don't match");
    CHECK_FATAL(U->n != mat->n || U->m != mat->m, "U dimensions don't match");

    const u32 n = mat->n;

    // 0 init matrices
    memset(L->data, 0, sizeof(int) * n * n);
    memset(U->data, 0, sizeof(int) * n * n);
    // L main diagonal is 1
    for (u32 i = 0; i < n; i++) { L->data[IDX(L, i, i)] = 1; }


    // Build U and L row by row
    for (u32 i = 0; i < n; i++) {
        // Upper triangular matrix U (row i, columns from i to n-1)
        for (u32 k = i; k < n; k++) {
            int sum = 0;
            for (u32 j = 0; j < i; j++) {
                sum += L->data[IDX(L, i, j)] * U->data[IDX(U, j, k)];
            }
            U->data[IDX(U, i, k)] = MATRIX_AT(mat, i, k) - sum;
        }

        // Lower triangular matrix L (column i, rows from i+1 to n-1)
        for (u32 k = i + 1; k < n; k++) {
            int sum = 0;
            for (u32 j = 0; j < i; j++) {
                sum += L->data[IDX(L, k, j)] * U->data[IDX(U, j, i)];
            }

            // Check for zero diagonal in U (would cause division by zero)
            if (U->data[IDX(U, i, i)] == 0) {
                CHECK_FATAL(1, "Matrix is singular - LU decomposition failed");
            }

            L->data[IDX(L, k, i)] =
                (MATRIX_AT(mat, k, i) - sum) / U->data[IDX(U, i, i)];
        }
    }
}

// LU Decomosition method
int matrix_det(const Matrix* mat)
{
    CHECK_FATAL(!mat, "mat matrix is null");
    CHECK_FATAL(mat->m != mat->n, "only square matrices have determinant");

    /*
        det of triangular mat is product of main diagonal
        so det of a mat that is decomposed with LU method becomes
        product of elements on the diagonal of L and U
        det(A) = det(L) * det(U)
        LU Decomosition is when we make 2 triangular matrices from one,
        which when xplied give original matrix
    */


    return 0;
}


void matrix_T(Matrix* out, const Matrix* mat)
{
    CHECK_FATAL(!mat, "mat matrix is null");
    CHECK_FATAL(!out, "out matrix is null");
    CHECK_FATAL(mat->m != out->n || mat->n != out->m,
                "incompatible matrix dimensions");

    // Block size for cache optimization (tune based on cache line size)
    const u32 BLOCK_SIZE = 16; // TODO: user adjustable macro?

    // Blocked transpose: process matrix in BLOCK_SIZE x BLOCK_SIZE tiles
    for (u32 i = 0; i < mat->m; i += BLOCK_SIZE) {
        for (u32 j = 0; j < mat->n; j += BLOCK_SIZE) {

            // Calculate block boundaries
            u32 i_max = (i + BLOCK_SIZE < mat->m) ? i + BLOCK_SIZE : mat->m;
            u32 j_max = (j + BLOCK_SIZE < mat->n) ? j + BLOCK_SIZE : mat->n;

            // Transpose the block
            for (u32 ii = i; ii < i_max; ii++) {
                for (u32 jj = j; jj < j_max; jj++) {
                    // mat[ii][jj] -> out[jj][ii]
                    out->data[IDX(out, jj, ii)] = mat->data[IDX(mat, ii, jj)];
                }
            }
        }
    }
}

static u32 digits(int x)
{
    u32 d = 0;
    if (x <= 0) {
        d = 1;                             // For '-' sign or '0'
        x = (x == INT_MIN) ? INT_MAX : -x; // Handle edge case
    }
    while (x > 0) {
        x /= 10;
        d++;
    }
    return d;
}

void matrix_print(const Matrix* mat)
{
    CHECK_FATAL(!mat, "matrix is null");

    u32 width = 0;
    u32 total = mat->m * mat->n;

    // Find max width - linear O(n * k)
    for (u32 i = 0; i < total; i++) {
        u32 d = digits(mat->data[i]); // O(k)
        if (d > width) { width = d; }
    }

    // Single linear loop O(n)
    for (u32 i = 0; i < total; i++) {
        // Print row separator
        if (i % mat->n == 0) {           // divide by columns to detect new row
            if (i > 0) { putchar('|'); } // Close previous row
            putchar('\n');
            putchar('|');
            putchar(' ');
        }

        // Print element
        printf("%-*d ", width, mat->data[i]);
    }

    // Close last row
    putchar('|');
    putchar('\n');
}



#include "matrix.h"
#include <string.h>
#include <stdio.h>

// ============================================================================
// IMPLEMENTATION MACROS
// ============================================================================

// Common helper function implementations
#define IMPLEMENT_MATRIX_CREATE(type) \
    Matrix_##type* matrix_create_##type(u32 m, u32 n) \
    { \
        CHECK_FATAL(n == 0 && m == 0, "n == m == 0"); \
        Matrix_##type* mat = (Matrix_##type*)malloc(sizeof(Matrix_##type)); \
        CHECK_FATAL(!mat, "matrix malloc failed"); \
        mat->m = m; \
        mat->n = n; \
        mat->data = (type*)malloc(sizeof(type) * n * m); \
        CHECK_FATAL(!mat->data, "matrix data malloc failed"); \
        return mat; \
    }

#define IMPLEMENT_MATRIX_CREATE_ARR(type) \
    Matrix_##type* matrix_create_arr_##type(u32 m, u32 n, const type* arr) \
    { \
        CHECK_FATAL(!arr, "input arr is null"); \
        Matrix_##type* mat = matrix_create_##type(m, n); \
        memcpy(mat->data, arr, sizeof(type) * m * n); \
        return mat; \
    }

#define IMPLEMENT_MATRIX_CREATE_STK(type) \
    void matrix_create_stk_##type(Matrix_##type* mat, u32 m, u32 n, type* data) \
    { \
        CHECK_FATAL(!mat, "matrix is null"); \
        CHECK_FATAL(!data, "data is null"); \
        mat->data = data; \
        mat->m = m; \
        mat->n = n; \
    }

#define IMPLEMENT_MATRIX_DESTROY(type) \
    void matrix_destroy_##type(Matrix_##type* mat) \
    { \
        CHECK_FATAL(!mat, "matrix is null"); \
        free(mat->data); \
        free(mat); \
    }

#define IMPLEMENT_MATRIX_SET_VAL(type) \
    void matrix_set_val_##type(Matrix_##type* mat, ...) \
    { \
        CHECK_FATAL(!mat, "matrix is null"); \
        va_list arr; \
        va_start(arr, mat); \
        u32 total = MATRIX_TOTAL(mat); \
        for (u32 i = 0; i < total; i++) { \
            mat->data[i] = (type)va_arg(arr, double); \
        } \
        va_end(arr); \
    }

// Continue with other implementations...

#define IMPLEMENT_MATRIX_PRINT(type, fmt) \
    void matrix_print_##type(const Matrix_##type* mat) \
    { \
        CHECK_FATAL(!mat, "matrix is null"); \
        for (u32 i = 0; i < mat->m; i++) { \
            printf("| "); \
            for (u32 j = 0; j < mat->n; j++) { \
                printf(fmt, mat->data[IDX(mat, i, j)]); \
                printf(" "); \
            } \
            printf("|\n"); \
        } \
    }

// ============================================================================
// COMPLETE IMPLEMENTATION FOR A SINGLE TYPE
// ============================================================================

#define IMPLEMENT_MATRIX_FULL(type, fmt) \
    IMPLEMENT_MATRIX_CREATE(type) \
    IMPLEMENT_MATRIX_CREATE_ARR(type) \
    IMPLEMENT_MATRIX_CREATE_STK(type) \
    IMPLEMENT_MATRIX_DESTROY(type) \
    IMPLEMENT_MATRIX_SET_VAL(type) \
    /* ... add other implementations ... */ \
    IMPLEMENT_MATRIX_PRINT(type, fmt)

// ============================================================================
// X-MACRO DISPATCHER FOR IMPLEMENTATIONS
// ============================================================================

// Generate implementations for all types
#define X(type, suffix, fmt) IMPLEMENT_MATRIX_FULL(type, fmt);
#include "matrix_types.def"
#undef X


