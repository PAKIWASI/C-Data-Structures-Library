#pragma once

#include "String.h"
#include "common.h"
#include "hashset.h"
#include "helpers.h"
#include "str_setup.h"


int hashset_test_1(void)
{
    // TODO: debug this shi 
    hashset* set = hashset_create(sizeof(String*), murmurhash3_str, str_cmp_ptr, str_copy_ptr,
                                  str_move_ptr, str_del_ptr, NULL);

    String* s = string_from_cstr("hello", NULL);
    hashset_insert(set, (const u8*)&s);

    hashset_print(set, str_print_ptr);

    string_destroy(s);
    hashset_destroy(set);
    return 0;
}

int hashset_test_2(void)
{
    hashset* set = hashset_create(sizeof(String), murmurhash3_str, str_cmp, str_copy, str_move, str_del, NULL);

    String* s = string_from_cstr("helo", NULL);
    hashset_insert(set, (const u8*)s);

    hashset_print(set, str_print);


    string_destroy(s);
    hashset_destroy(set);
    return 0;
}

