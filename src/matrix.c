#include "matrix.h"

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Index calculation (row major)
#define IDX(mat, i, j) (((i) * (mat)->m) + (j))
#define MATRIX_AT(mat, i, j) ((mat)->data[(i)*(mat)->m + (j)])


Matrix* matrix_create(u32 n, u32 m)
{
    CHECK_FATAL(n == 0 && m == 0, "n == m == 0");

    Matrix* mat = (Matrix*)malloc(sizeof(Matrix));
    CHECK_FATAL(!mat, "matrix malloc failed");

    mat->n    = n;
    mat->m    = m;
    mat->data = (int*)malloc(sizeof(int) * n * m);
    CHECK_FATAL(!mat->data, "matrix data malloc failed");

    return mat;
}

Matrix* matrix_create_arr(u32 n, u32 m, const int* arr)
{
    CHECK_FATAL(!arr, "input arr is null");
    Matrix* mat = matrix_create(n, m);
    memcpy(mat->data, arr, sizeof(int) * n * m);
    return mat;
}

void matrix_create_stk(Matrix* mat, u32 n, u32 m, int* data)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!data, "data is null");

    mat->data = data;   // we can do this on the stack
    mat->n    = n;
    mat->m    = m;
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
    u32 n = MATRIX_TOTAL(mat);
    for (u32 i = 0; i < n; i++) {
        mat->data[i] = va_arg(arr, int);
    }
    va_end(arr);
}

void matrix_set_val_arr(Matrix* mat, const int* arr, u32 count)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(count != MATRIX_TOTAL(mat), "count doesn't match matrix size");

    memcpy(mat->data, arr, sizeof(int) * count);
}

void matrix_set_val_arr2(Matrix* mat, const int** arr2, u32 n, u32 m)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!arr2, "arr is null");
    CHECK_FATAL(!*arr2, "*arr is null");
    CHECK_FATAL(n != mat->n || m != mat->m, "mat dimentions dont match passed arr2");

    u32 idx = 0;
    for (u32 i = 0; i < n; i++) {
        memcpy(mat->data + idx, arr2[i], sizeof(int) * m); 
        idx += m;
    }
}

void matrix_set_elm(Matrix* mat, int elm, u32 i, u32 j)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(i >= mat->n || j >= mat->m, "index out of bounds");

    mat->data[IDX(mat, i, j)] = elm;
}

void matrix_add(Matrix* out, const Matrix* a, const Matrix* b)
{
    CHECK_FATAL(!out, "out matrix is null");
    CHECK_FATAL(!a,   "a matrix is null");
    CHECK_FATAL(!b,   "b matrix is null");
    CHECK_FATAL(a->n != b->n || a->m != b->m || a->n != out->n || a->m != out->m,
                "a, b, out mat dimentions dont match");

    u32 total = MATRIX_TOTAL(a);

    for (u32 i = 0; i < total; i++) {
        out->data[i] = a->data[i] + b->data[i];
    }
}


void matrix_sub(Matrix* out, const Matrix* a, const Matrix* b)
{
    CHECK_FATAL(!out, "out matrix is null");
    CHECK_FATAL(!a,   "a matrix is null");
    CHECK_FATAL(!b,   "b matrix is null");
    CHECK_FATAL(a->n != b->n || a->m != b->m, "a, b mat dimentions dont match");

    u32 total = MATRIX_TOTAL(a);

    for (u32 i = 0; i < total; i++) {
        out->data[i] = a->data[i] - b->data[i];
    }
}


void matrix_xply(Matrix* out, const Matrix* a, const Matrix* b)
{
    CHECK_FATAL(!out, "out matrix is null");
    CHECK_FATAL(!a,   "a matrix is null");
    CHECK_FATAL(!b,   "b matrix is null");

    u32 total = MATRIX_TOTAL(a);


}

void matrix_xply_self(Matrix* a, const Matrix* b)
{
    CHECK_FATAL(!a,   "a matrix is null");
    CHECK_FATAL(!b,   "b matrix is null");

    u32 total = MATRIX_TOTAL(a);

}


static u32 digits(int x)
{
    u32 d = 0;
    if (x <= 0) {
        d = 1;  // For '-' sign or '0'
        x = (x == INT_MIN) ? INT_MAX : -x;  // Handle edge case
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
    u32 total = mat->n * mat->m;

    // Find max width - linear O(n * k)
    for (u32 i = 0; i < total; i++) {
        u32 d = digits(mat->data[i]); // O(k)
        if (d > width) { width = d; }
    }

    // Single linear loop O(n)
    for (u32 i = 0; i < total; i++) {
        // Print row separator
        if (i % mat->m == 0) {
            if (i > 0) { putchar('|'); }  // Close previous row
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



