#include "String.h"
#include <stdio.h>



// TEST 1: string creating and append
int test_String_1(void)
{
    String* s1 = string_create();

    printf("%d\n", string_empty(s1));
    string_append_char(s1, 'a');
    string_append_char(s1, 'a');
    string_append_char(s1, 'a');

    string_print(s1);
    printf("\n");

    printf("%c\n", string_pop_char(s1));
    string_print(s1);
    printf("\n");

    string_append_cstr(s1, "hello");

    string_print(s1);
    printf("\n");

    String* s2 = string_from_string(s1);
    string_print(s2);
    printf("\n");
    
    string_reserve(s2, 10);
    string_append_string(s2, s1);

    string_print(s1);
    printf("\n");
    string_print(s2);
    printf("\n");

    String s3;
    string_create_onstk(&s3, string_to_cstr(s1));
    
    string_append_cstr(&s3, " hwatt");
    string_print(&s3);

    string_destroy(s1);
    string_destroy(s2);
    string_destroy_fromstk(&s3);
    return 0;
}


// TEST 2: find, insert etc
int test_String_2(void)
{
    String* s1 = string_from_cstr("wasi ullah satti"); 

    size_t i = string_find_char(s1, ' '); 
    string_insert_cstr(s1, i, " whar");
    
    string_print(s1);

    string_destroy(s1);
    return 0;
}
