#include "String.h"
#include <stdio.h>




int string_test_1(void)
{
    String* s1 = string_create(); 
    string_append_cstr(s1, "this is s1");
    string_print(s1);
    printf("\n");
    string_destroy(s1);

    String s2;
    string_create_stk(&s2, "this is s2");
    string_print(&s2);
    printf("\n");
    string_destroy_stk(&s2);

    String* s3 = string_from_cstr("this is s3");
    string_print(s3);
    printf("\n");
    String* s4 = string_from_string(s3);
    string_destroy(s3);

    string_reserve_char(s4, 20, 'x');
    string_print(s4);
    printf("\n");
    String s5;
    string_copy(&s5, s4);
    string_destroy(s4);

    string_print(&s5);
    printf("\n");
    string_destroy_stk(&s5);

    return 0;
}


