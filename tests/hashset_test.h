#pragma once

#include "String.h"
#include "common.h"
#include "hashset.h"
#include "helpers.h"
#include "str_setup.h"


int hashset_test_1(void)
{
    hashset* set = hashset_create(sizeof(int), NULL, NULL, NULL, NULL, NULL);

    int a = 5; 
    hashset_insert(set, cast(a));
    hashset_insert(set, cast(a));
    hashset_insert(set, cast(a));
    hashset_insert(set, cast(a));
    hashset_insert(set, cast(a));
    hashset_insert(set, cast(a));
    hashset_insert(set, cast(a));

    hashset_print(set, int_print);

    hashset_destroy(set);
    return 0;
}

int hashset_test_2(void)
{
    hashset* set = hashset_create(sizeof(String), 
            murmurhash3_str, str_cmp, str_copy, str_move, str_del);

    String* s1 = string_from_cstr("hello");
    String* s2 = string_from_cstr("hollo");

    hashset_insert_move(set, (u8**)&s1);
    hashset_insert_move(set, (u8**)&s2);

    hashset_print(set, str_print);

    String* s3 = string_from_cstr("helllo");
    if (!hashset_remove(set, (u8*)s3)) {
        hashset_insert(set, (u8*)s3);
    } 
    hashset_print(set, str_print);


    if (!hashset_remove(set, (u8*)s3)) {
        hashset_insert_move(set, (u8**)&s3);
    } else {
        string_destroy(s3);
    }

    hashset_print(set, str_print);

    hashset_destroy(set);
    return 0;
}


