#pragma once

#include "String.h"
#include "common.h"
#include "hashset.h"
#include "helpers.h"
#include "str_setup.h"


int hashset_test_1(void)
{
    hashset* set = hashset_create(sizeof(String*), murmurhash3_str_ptr, str_cmp_ptr, str_copy_ptr,
                                  str_move_ptr, str_del_ptr);

    String* s = string_from_cstr("hello");
    hashset_insert(set, cast(s), false);

    hashset_print(set, str_print_ptr);

    string_destroy(s);
    hashset_destroy(set);
    return false;
}
