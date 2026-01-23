#include "String.h"




int string_test_1(void)
{
    String* str = string_create();
    string_append_cstr(str, "hello");
    string_print(str);
    printf("\n");

    String* s = string_from_string(str);
    string_append_char(s, ' ');
    string_append_string(s, str);
    string_print(s);
    printf("\n");

    String s2;
    string_copy(&s2, s);
    string_append_cstr(&s2, " wasi");
    string_print(&s2);
    printf("\n");

    String* s3 = string_from_cstr("wtf");
    string_move(s3, &str);
    string_print(s3);
    printf("\n");

    string_append_string_move(s3, &s);
    string_print(s3);
    printf("\n");


    //string_destroy(str);
    //string_destroy(s);
    string_destroy_stk(&s2);
    string_destroy(s3);

    return 0;
}


