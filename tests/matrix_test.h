#ifndef MATRIX_TEST_H
#define MATRIX_TEST_H

#include "helpers.h"
#include "matrix.h"


int matrix_test_1(void)
{
    int data[3 * 3] = {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1,
    };
    Matrix mat;

    matrix_create_stk(&mat, (u8*)data, 3, 3);

    matrix_set_val(&mat, 2, 2, 2,
                         2, 2, 2,
                         2, 2, 2);           

    matrix_print(&mat, int_print);

    return 0;
}



#endif // MATRIX_TEST_H
