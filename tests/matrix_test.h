#ifndef MATRIX_TEST_H
#define MATRIX_TEST_H

#include "matrix.h"
#include "matrix_generic.h"


/*
int matrix_test_1(void)
{
    Matrix mat;
    int data[3][4] = {
        {1, 1, 1, 1},
        {2, 2, 2, 2},
        {3, 3, 3, 3}
    };
    matrix_create_stk(&mat, 3, 4, (int*)data);

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

    matrix_set_val_arr(mat, 9, (int[9]){
            1, 1, 1,
            2, 2, 2,
            3, 3, 3
    });

    matrix_print(mat);

    // prefer this as much more explicit, hard to make mistake
    matrix_set_val_arr(mat, 9, (int*)(int[3][3]){
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
    Matrix* mat = matrix_create_arr(4, 3, (int*)(int[4][3]){
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1},
    });

    matrix_print(mat);

    Matrix mat2;
    matrix_create_stk(&mat2, 4, 3, (int*)(int[4][3]){
        {2, 2, 2},
        {2, 2, 2},
        {2, 2, 2},
        {2, 2, 2},
    });

    matrix_print(&mat2);

    matrix_add(mat, mat, &mat2);

    matrix_print(mat);

    Matrix out;
    matrix_create_stk(&out, 4, 3, (int*)ZEROS_2D(4, 3));
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
    matrix_create_stk(&mat, 4, 3, (int*)(int[4][3]){
        {1, 2, 3},
        {1, 2, 3},
        {1, 2, 3},
        {1, 2, 3},
    });
    matrix_print(&mat);

    Matrix m2;
    matrix_create_stk(&m2, 3, 4, (int*)ZEROS_2D(3, 4));

    matrix_T(&m2, &mat);
    matrix_print(&m2);

    Matrix m3;
    matrix_create_stk(&m3, 3, 3, (int*)ZEROS_2D(3, 3));
    matrix_xply(&m3, &m2, &mat);

    matrix_print(&m3);

    return 0;
}

int matrix_test_5(void)
{
    Matrix* mat = matrix_create_arr(3, 3, (int*)(int[3][3]){
        {3, 2, 4},
        {2, 0, 2},      // BUG: not working for 0 in middle (saying it's singular!?)
        {4, 2, 3},
    });

    Matrix L, U;
    matrix_create_stk(&L, 3, 3, (int*)ZEROS_2D(3, 3));
    matrix_create_stk(&U, 3, 3, (int*)ZEROS_2D(3, 3));

    matrix_LU_Decomp(&L, &U, mat);  // WARN: L and U should be floats!

    matrix_print(&L);
    matrix_print(&U);

    matrix_destroy(mat);
    return 0;
}
*/

int matrix_test_6(void)
{
    // Example 1: Working with double matrices for numerical computation
    printf("=== Solving Linear System with LU Decomposition ===\n\n");
    
    // Create coefficient matrix A
    Matrix_double* A = matrix_create_double(3, 3);
    matrix_set_val_arr_double(A, 9, (double*)(double[3][3]){
        {2.0,  1.0, -1.0},
        {-3.0, -1.0, 2.0},
        {-2.0, 1.0,  2.0}
    });
    
    printf("Coefficient Matrix A:\n");
    matrix_print_double(A);
    
    // Perform LU decomposition
    Matrix_double* L = matrix_create_double(3, 3);
    Matrix_double* U = matrix_create_double(3, 3);
    matrix_LU_Decomp_double(L, U, A);
    
    printf("\nLower Triangular Matrix L:\n");
    matrix_print_double(L);
    
    printf("\nUpper Triangular Matrix U:\n");
    matrix_print_double(U);
    
    // Calculate determinant
    double det = matrix_det_double(A);
    printf("\nDeterminant of A: %.6lf\n", det);
    
    // Verify: L * U = A
    Matrix_double* verify = matrix_create_double(3, 3);
    matrix_xply_double(verify, L, U);
    printf("\nVerification (L × U):\n");
    matrix_print_double(verify);
    
    // Cleanup
    matrix_destroy_double(A);
    matrix_destroy_double(L);
    matrix_destroy_double(U);
    matrix_destroy_double(verify);
    
    
    // Example 2: Integer matrix operations
    printf("\n\n=== Integer Matrix Operations ===\n\n");
    
    Matrix_int* M1 = matrix_create_int(2, 3);
    Matrix_int* M2 = matrix_create_int(2, 3);
    
    matrix_set_val_arr_int(M1, 6, (int*)(int[2][3]){
        {1, 2, 3},
        {4, 5, 6}
    });
    
    matrix_set_val_arr_int(M2, 6, (int*)(int[2][3]){
        {6, 5, 4},
        {3, 2, 1}
    });
    
    printf("Matrix M1:\n");
    matrix_print_int(M1);
    
    printf("\nMatrix M2:\n");
    matrix_print_int(M2);
    
    // Addition
    Matrix_int* sum = matrix_create_int(2, 3);
    matrix_add_int(sum, M1, M2);
    printf("\nM1 + M2:\n");
    matrix_print_int(sum);
    
    // Subtraction
    Matrix_int* diff = matrix_create_int(2, 3);
    matrix_sub_int(diff, M1, M2);
    printf("\nM1 - M2:\n");
    matrix_print_int(diff);
    
    // Transpose
    Matrix_int* M1_T = matrix_create_int(3, 2);
    matrix_T_int(M1_T, M1);
    printf("\nTranspose of M1:\n");
    matrix_print_int(M1_T);
    
    // Matrix multiplication: M1 (2×3) × M1_T (3×2) = (2×2)
    Matrix_int* product = matrix_create_int(2, 2);
    matrix_xply_int(product, M1, M1_T);
    printf("\nM1 × M1^T:\n");
    matrix_print_int(product);
    
    // Cleanup
    matrix_destroy_int(M1);
    matrix_destroy_int(M2);
    matrix_destroy_int(sum);
    matrix_destroy_int(diff);
    matrix_destroy_int(M1_T);
    matrix_destroy_int(product);
    
    
    // Example 3: Stack allocation for temporary work
    printf("\n\n=== Stack Allocation Example ===\n\n");
    
    Matrix_float stack_mat;
    float temp_data[2][2] = {
        {1.5f, 2.5f},
        {3.5f, 4.5f}
    };
    
    matrix_create_stk_float(&stack_mat, 2, 2, (float*)temp_data);
    printf("Stack Matrix (no heap allocation):\n");
    matrix_print_float(&stack_mat);
    
    // Scale it
    matrix_scale_float(&stack_mat, 10.0f);
    printf("\nAfter scaling by 10:\n");
    matrix_print_float(&stack_mat);
    
    // No destroy needed for stack matrices!
    
    return 0;
}


#endif // MATRIX_TEST_H
