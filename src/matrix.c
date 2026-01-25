#include "matrix.h"
#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Index calculation (row major)
#define IDX(mat, i, j) (((i) * (mat)->m) + (j))
#define GET_ELM(mat, i, j) (mat->data[IDX(mat, i, j)])


Matrix* matrix_create(u32 n, u32 m)
{
    CHECK_FATAL(n == 0 && m == 0, "n == m == 0");

    Matrix* mat = (Matrix*)malloc(sizeof(Matrix));
    CHECK_FATAL(!mat, "matrix malloc failed");

    mat->n    = n;
    mat->m    = m;
    mat->data = malloc(sizeof(int) * n * m);
    CHECK_FATAL(!mat->data, "matrix data malloc failed");

    return mat;
}

void matrix_create_stk(Matrix* mat, int* data, u32 n, u32 m)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!data, "data is null");

    mat->data = data;
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



static u32 digits(int x)
{
    u32 d = (x <= 0); // handles 0 and negatives
    while (x) {
        x /= 10;
        d++;
    }
    return d;
}

void matrix_print(Matrix* mat)
{
    CHECK_FATAL(!mat, "matrix is null");

    u32 width = 0;

    // find max width
    for (u32 i = 0; i < mat->n; i++) {
        for (u32 j = 0; j < mat->m; j++) {
            u32 d = digits(GET_ELM(mat, i, j));
            if (d > width) { width = d; }
        }
    }

    for (u32 i = 0; i < mat->n; i++) {
        putchar('|');
        putchar(' ');

        for (u32 j = 0; j < mat->m; j++) {
            printf("%-*d ", width, GET_ELM(mat, i, j));
        }

        putchar('|');
        putchar('\n');
    }
    putchar('\n');
}



