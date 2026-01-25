#ifndef MATRIX_TEST_H
#define MATRIX_TEST_H

#include "matrix.h"


int matrix_test_1(void)
{
    Matrix mat;
    int data[3][4] = {
        {1, 1, 1, 1},
        {2, 2, 2, 2},
        {3, 3, 3, 3}
    };
    matrix_create_stk(&mat, (int*)data, 3, 4);

    matrix_print(&mat);

    matrix_set_val(&mat, 2, 2, 2, 2,
                         2, 2, 2, 2,
                         2, 2, 2, 2);           

    matrix_set_elm(&mat, 0, 1, 1);

    matrix_print(&mat);


    return 0;
}

int matrix_test_2(void)
{
    Matrix* mat = matrix_create(3, 3);

    matrix_set_val_arr(mat, (int[9]){
            1, 1, 1,
            2, 2, 2,
            3, 3, 3
    }, 9);

    matrix_print(mat);

    // prefer this as much more explicit, hard to make mistake
    matrix_set_val_arr(mat, (int*)(int[3][3]){
        {1, 200, 3},
        {4, 5, 6},
        {70, 8, 8}
    }, 9);

    matrix_print(mat);

    matrix_destroy(mat);

    return 0;
}



#endif // MATRIX_TEST_H
