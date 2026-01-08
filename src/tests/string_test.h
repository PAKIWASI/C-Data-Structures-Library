#pragma once

#include "String.h"



int string_test_1(void)
{
    String* str = string_from_cstr("hello");

    string_print(str);
    printf("\n");

    string_append_char(str, 'W');
    string_print(str);
    printf("\n");

    string_append_cstr(str, "orld");
    string_print(str);
    printf("\n");

    String s2;
    string_create_onstk(&s2, " what");

    string_append_string(str, &s2);


    string_print(str);
    printf("\n");

    String* s3 = string_from_string(&s2);
    string_append_cstr(s3, "wasiii");

    string_print(&s2);
    printf("\n");
    string_print(s3);
    printf("\n");

    string_pop_char(s3);
    string_pop_char(s3);
    string_print(s3);
    printf("\n");

    u32 pos = string_find_cstr(str, "what");
    string_insert_cstr(str, pos, "fuck ");

    string_print(str);
    printf("\n");

    string_remove_range(str, pos, 99);

    string_print(str);
    printf("\n");

    string_clear(&s2);
    string_clear(s3);

    string_append_cstr(&s2, "hell");
    string_append_cstr(s3, "helli");

    printf("%d\n", string_compare(s3, &s2));

    string_destroy(str);
    string_destroy_fromstk(&s2);
    string_destroy(s3);
    return 0;
}
