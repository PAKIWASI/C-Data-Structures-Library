#include "String.h"
#include "gen_vector.h"
#include "helper_functions.h"
#include "str_setup.h"





int test_genVec_1(void)
{
    // TEST 1: simple vector init with basic datatype
    genVec* vec1 = genVec_init(3, sizeof(double), NULL);

    vec_push_double(vec1, 1);
    vec_push_double(vec1, 1.1111111);
    vec_push_double(vec1, 1);
    vec_push_double(vec1, 1);
    vec_push_double(vec1, 1);
    vec_push_double(vec1, 5);
    vec_push_double(vec1, 10.000001);

    genVec_print(vec1, double_print);

    printf("%f\n", vec_pop_double(vec1));
    printf("%f\n", vec_pop_double(vec1));
    printf("%f\n", vec_pop_double(vec1));
    printf("%f\n", vec_pop_double(vec1));

    genVec_print(vec1, double_print);

    printf("%f\n", *(double*)genVec_get_ptr(vec1, 1));
    *(double*)genVec_get_ptr(vec1, 1) += 1;
    printf("%f\n", *(double*)genVec_get_ptr(vec1, 1));

    genVec_print(vec1, double_print);
    
    genVec_clear(vec1);

    genVec_print(vec1, double_print);

    genVec_reserve(vec1, 10);
    vec_push_double(vec1, 6.7);
    vec_push_double(vec1, 6.7);
    vec_push_double(vec1, 6.7);

    genVec_print(vec1, double_print);

    double d = 8;
    genVec_reserve_val(vec1, 10, cast(d));

    genVec_print(vec1, double_print);

    genVec_clear(vec1);

    genVec_reserve_val(vec1, 29, cast(d));

    genVec_print(vec1, double_print);

    genVec_destroy(vec1);

    return 0;
}

void vec_push_str(genVec* vec, const char* cstr) {
    String str;
    string_create_onstack(&str, cstr);
    genVec_push(vec, cast(str));
    // if destroy from stk : buffer gets deleted
}

int test_genVec_2(void)
{
    genVec vec;
    genVec_init_stk(10, sizeof(String), string_custom_delete, &vec);

    vec_push_str(&vec, "hello"); 
    vec_push_str(&vec, "workd"); 
    vec_push_str(&vec, "fjdfdf"); 
    vec_push_str(&vec, "he"); 
    vec_push_str(&vec, "llo"); 
    vec_push_str(&vec, ""); 

    genVec_print(&vec, str_print);

    string_print((const String*)genVec_get_ptr(&vec, 2));
    printf("\n");
    string_append_cstr((String*)genVec_get_ptr(&vec, 2), "WASI");

    genVec_print(&vec, str_print);

    genVec_destroy_stk(&vec);

    return 0;
}
