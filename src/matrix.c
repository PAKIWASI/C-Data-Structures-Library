#include "matrix.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>


// Index calculation (row major) // TODO: check
#define IDX(mat, i, j) (((i) * (mat)->m) + (j))




Matrix* matrix_create(u32 n, u32 m)
{
    CHECK_FATAL(n == 0 && m == 0, "n == m == 0");

    Matrix* mat = (Matrix*)malloc(sizeof(Matrix));
    CHECK_FATAL(!mat, "matrix malloc failed");

    mat->n = n;
    mat->m = m;
    mat->data = malloc(MATRIX_TOTAL(mat));
    CHECK_FATAL(!mat->data, "matrix data malloc failed");

    return mat;
}

void matrix_create_stk(Matrix* mat, u8* data, u32 n, u32 m)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!data, "data is null");

    mat->data = data;
    mat->n = n;
    mat->m = m;
}

void matrix_destroy(Matrix* mat)
{
    CHECK_FATAL(!mat, "matrix is null");

    free(mat->data);
    free(mat);
}


void matrix_set_val(Matrix* mat, ...)
{

}


void matrix_print(Matrix* mat, matrix_print_fn print_fn)
{
    CHECK_FATAL(!mat, "matrix is null");
    CHECK_FATAL(!print_fn, "print_fn is null");
    
    for (u32 i = 0; i < mat->n; i++) {
        for (u32 j = 0; j < mat->m; j++) {
            print_fn((const u8*)&mat->data[IDX(mat, i, j)]);
            putchar(' ');
        }
        putchar('\n');
    }
    putchar('\n');
}
