#include "String.h"
#include "common.h"
#include "gen_vector.h"
#include "helpers.h"
#include <string.h>



// test push (copy)
int genVec_test_1(void)
{
    genVec* vec = genVec_init(10, sizeof(String), str_copy, str_move, str_del);

    String* str = string_from_cstr("hello");
    genVec_push(vec, castptr(str));
    genVec_push(vec, castptr(str));

    string_append_cstr(str, " what is up");
    genVec_push(vec, castptr(str));
    genVec_push(vec, castptr(str));

    string_print(str);
    string_destroy(str);

    // Initialize v2 before copying
    genVec v2;
    genVec_copy(&v2, vec);

    genVec_print(vec, str_print);
    genVec_print(&v2, str_print);
    
    genVec_destroy(vec);
    genVec_destroy_stk(&v2);

    return 0;
}

// test push_move (transfer ownership)
int genVec_test_2(void)
{
    genVec* vec = genVec_init(10, sizeof(String), str_copy, str_move, str_del);

    String* str = string_from_cstr("hello");
    string_print(str);
    printf("\n");

    genVec_push_move(vec, (u8**)&str);

    // Move heap vector to stack variable
    genVec v2;
    genVec_move(&v2, &vec);  // vec is now NULL

    genVec_print(&v2, str_print);
    genVec_destroy_stk(&v2);

    return 0;
}


// === test vec of string* (sizeof(String*)) ===
//==============================================

// when we try to copy elements from one vec to another, then src is also string**
// so we take input as string**, always
// so when we create stack str, then we have to assign ptr to it and pass address of that


// test push (copy) 
int genVec_test_3(void)
{
    genVec* vec = genVec_init(10, sizeof(String*), str_copy_ptr, str_move_ptr, str_del_ptr);

    String str;
    u8* p = (u8*)&str;
    string_create_stk((String*)p, "hello");
    genVec_push(vec, (const u8*)&p); //  address of pointer to str (double ptr)

    string_print((String*)p);
    string_destroy_stk((String*)p);

    genVec v2;
    genVec_copy(&v2, vec);

    genVec_print(vec, str_print_ptr);
    genVec_print(&v2, str_print_ptr);

    genVec_destroy(vec);
    genVec_destroy_stk(&v2);

    return 0;
}

// test push_move 
int genVec_test_4(void)
{
    genVec* vec = genVec_init(10, sizeof(String*), str_copy_ptr, str_move_ptr, str_del_ptr);

    String* str = string_from_cstr("hello");
    genVec_push_move(vec, (u8**)&str); // take String** for move

    genVec* v2 = malloc(sizeof(genVec));
    genVec_move(v2, &vec);

    
    genVec_print(v2, str_print_ptr);
    genVec_destroy(v2);

    return 0;
}
    



// === testing macros  ===
//==============================================

int genVec_test_5(void)
{
    genVec vec;
    genVec_init_stk(1000, sizeof(float), NULL, NULL, NULL, &vec);

    for (int i = 0; i < 1000; i++) {
        VEC_PUSH_SIMP(&vec, float, (float)i + 0.1f);
    }

    genVec_print(&vec, float_print);

    genVec_destroy_stk(&vec);
    return 0;
}

int genVec_test_6(void)
{
    genVec* vec = genVec_init(10, sizeof(String*), str_copy_ptr, str_move_ptr, str_del_ptr);

    VEC_PUSH_CSTR(vec, "hello");
    VEC_PUSH_CSTR(vec, "hello");

    genVec_print(vec, str_print_ptr);

    genVec_destroy(vec);
    return 0;
}


