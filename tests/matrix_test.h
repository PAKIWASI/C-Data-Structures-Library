#ifndef MATRIX_TEST_H
#define MATRIX_TEST_H

#include "matrix.h"


int matrix_test_1(void)
{
    Matrix mat;
    float data[3][4] = {
        {1, 1, 1, 1},
        {2, 2, 2, 2},
        {3, 3, 3, 3}
    };
    matrix_create_stk(&mat, 3, 4, (float*)data);

    matrix_print(&mat);

    // matrix_set_val(&mat, 2, 2, 2, 2,
    //                      2, 2, 2, 2,
    //                      2, 2, 2, 2);           

    matrix_set_elm(&mat, 0, 1, 1);

    matrix_print(&mat);


    return 0;
}

int matrix_test_2(void)
{
    Matrix* mat = matrix_create(3, 3);

    matrix_set_val_arr(mat, 9, (float[9]){
            1, 1, 1,
            2, 2, 2,
            3, 3, 3
    });

    matrix_print(mat);

    // prefer this as much more explicit, hard to make mistake
    matrix_set_val_arr(mat, 9, (float*)(float[3][3]){
        {1,  200, 3},
        {4,  5,   6},
        {70, 8,   8}
    });

    matrix_print(mat);

    matrix_destroy(mat);

    return 0;
}

int matrix_test_3(void)
{
    Matrix* mat = matrix_create_arr(4, 3, (float*)(float[4][3]){
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1},
    });

    matrix_print(mat);

    Matrix mat2;
    matrix_create_stk(&mat2, 4, 3, (float*)(float[4][3]){
        {2, 2, 2},
        {2, 2, 2},
        {2, 2, 2},
        {2, 2, 2},
    });

    matrix_print(&mat2);

    matrix_add(mat, mat, &mat2);

    matrix_print(mat);

    Matrix out;
    matrix_create_stk(&out, 4, 3, (float*)ZEROS_2D(4, 3));
    matrix_print(&out);

    matrix_sub(&out, mat, &mat2);
    matrix_print(&out);


    matrix_sub(&out, &out, &out);
    matrix_print(&out);


    matrix_destroy(mat);
    return 0;
}

int matrix_test_4(void)
{
    Matrix mat;
    matrix_create_stk(&mat, 4, 3, (float*)(float[4][3]){
        {1, 2, 3},
        {1, 2, 3},
        {1, 2, 3},
        {1, 2, 3},
    });
    matrix_print(&mat);

    Matrix m2;
    matrix_create_stk(&m2, 3, 4, (float*)ZEROS_2D(3, 4));

    matrix_T(&m2, &mat);
    matrix_print(&m2);

    Matrix m3;
    matrix_create_stk(&m3, 3, 3, (float*)ZEROS_2D(3, 3));
    matrix_xply(&m3, &m2, &mat);

    matrix_print(&m3);

    return 0;
}

int matrix_test_5(void)
{
    Matrix* mat = matrix_create_arr(3, 3, (float*)(float[3][3]){
        {3, 2, 4},
        {2, 0, 2},      // BUG: not working for 0 in middle (saying it's singular!?)
        {4, 2, 3},
    });

    Matrix L, U;
    matrix_create_stk(&L, 3, 3, (float*)ZEROS_2D(3, 3));
    matrix_create_stk(&U, 3, 3, (float*)ZEROS_2D(3, 3));

    matrix_LU_Decomp(&L, &U, mat);  // WARN: L and U should be floats!

    matrix_print(&L);
    matrix_print(&U);

    matrix_destroy(mat);
    return 0;
}

#include "matrix_generic.h"
INSTANTIATE_MATRIX(double, "%lf");

int matrix_test_6(void)
{
}


#endif // MATRIX_TEST_H
