#include "String.h"
#include "gen_vector.h"
#include "helper_functions.h"
#include "str_setup.h"

#include <stddef.h>
#include <string.h>





// TEST 1: simple vector init with basic datatype
int test_genVec_1(void)
{
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

    genVec_reserve_val(vec1, 5, cast(d));

    vec_replace_double(vec1, 2, 0.4234); 
    printf("%f\n", vec_get_double(vec1, 2));

    genVec_print(vec1, double_print);

    vec_insert_double(vec1, 4, 74747.7);

    genVec_print(vec1, double_print);

    double arr[10] = {0};
    genVec_insert_multi(vec1, genVec_size(vec1), (const u8*)arr, 10);
    genVec_print(vec1, double_print);

    // test new remove range function
    genVec_remove_range(vec1, 1, 4);
    
    genVec_print(vec1, double_print);

    genVec_destroy(vec1);

    return 0;
}


// TEST 2: vec on stack with string data
int test_genVec_2(void)
{
    genVec vec;
    genVec_init_stk(10, sizeof(String), string_custom_delete, &vec);


    vec_push_cstr(&vec, "hello"); 
    vec_push_cstr(&vec, "workd"); 
    vec_push_cstr(&vec, "fjdfdf"); 
    vec_push_cstr(&vec, "he"); 
    vec_push_cstr(&vec, "llo"); 
    vec_push_cstr(&vec, ""); 
    vec_push_cstr(&vec, "wtf"); 

    genVec_print(&vec, str_print);

    string_print((const String*)genVec_get_ptr(&vec, 2));
    printf("\n");
    string_append_cstr((String*)genVec_get_ptr(&vec, 2), "WASI");

    genVec_print(&vec, str_print);

    String* s1 = vec_pop_str(&vec);
    string_print(s1);
    string_destroy(s1);
    printf("\n");

    s1 = vec_pop_str(&vec);
    string_print(s1);
    string_destroy(s1);
    printf("\n");

    printf("%s\n", vec_get_cstr(&vec, 1));

    string_print((const String*)genVec_front(&vec));
    printf("\n");
    string_print((const String*)genVec_back(&vec));
    printf("\n");
    
    /*
    // shallow print (points to same data)
    genVec* copy = genVec_copy(&vec);

    string_append_cstr((String*)genVec_get_ptr(copy, 0), "world");

    genVec_print(&vec, str_print);
    genVec_print(copy, str_print);

    // test new remove range
    genVec_remove_range(copy, 1, 1);
    genVec_print(copy, str_print);

    //genVec_destroy_stk(&vec);  // double free
    genVec_destroy(copy); // data destroyed

    */
    return 0;
}



// Custom delete function for vector elements that are themselves vectors
static inline void vec_custom_del(u8* elm) {
    genVec* vec = (genVec*)elm;
    genVec_destroy_stk(vec); // Use destroy_stk since we used init_stk
    // we never malloced so can't free
}// the vec->data block contains pointers to malloced data, and other metadata
// that can only be freed by free(vec), can't individually free each!



int test_genVec_3(void)
{
    genVec vec;
    genVec_init_stk(10, sizeof(genVec), vec_custom_del, &vec);
    
    for (int i = 0; i < 10; i++) {
        genVec v;
        genVec_init_stk(5, sizeof(int), NULL, &v);
        for (int j = 0; j < 5; j++) {
            genVec_push(&v, cast(j));
        } 
        genVec_push(&vec, cast(v));
    }
    
    // Print the structure
    genVec_print(&vec, vecvecint_print);
    
    genVec* arr = (genVec*)genVec_get_ptr(&vec, 1);
    vec_push_ints(arr, 1,2,3,4,5,5,6);
    genVec_print(&vec, vecvecint_print);

    // Cleanup - this will call vec_custom_del on each element
    genVec_destroy_stk(&vec);
    return 0;
}

u8* str_copy(const u8* elm) {
    return (u8*)string_from_string((const String*)elm);
}

int test_genVec_4(void)
{
    genVec vec;
    genVec_init_stk(0, sizeof(String), string_custom_delete, &vec);


    genVec_destroy_stk(&vec);

    return 0;
}

