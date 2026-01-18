#include "String.h"
#include "common.h"
#include "hashmap.h"
#include "helpers.h"
#include <stdio.h>



// test push (copy) - string stored as value
int hashmap_test_1(void)
{
    hashmap* map = hashmap_create(sizeof(int), sizeof(String), NULL, NULL, NULL, str_copy,
                                  NULL, str_move, NULL, str_del);

    int a = 5;
    String str;
    string_create_stk(&str, "hello");

    hashmap_put(map, cast(a), 0, cast(str), 0);
    a++;
    hashmap_put(map, cast(a), 0, cast(str), 0);
    a++;
    hashmap_put(map, cast(a), 0, cast(str), 0);
    a++;
    hashmap_put(map, cast(a), 0, cast(str), 0);
    a++;
    hashmap_put(map, cast(a), 0, cast(str), 0);
    a++;
    hashmap_put(map, cast(a), 0, cast(str), 0);
    a++;
    hashmap_put(map, cast(a), 0, cast(str), 0);

    hashmap_print(map, int_print, str_print);

    String* s = (String*)hashmap_get_ptr(map, cast(a));
    string_append_cstr(s, " waht is up");
    string_print(s);
    printf("\n");
    s = NULL;

    hashmap_print(map, int_print, str_print);
    
    String s2 = {0};
    hashmap_get(map, cast(a), cast(s2));
    string_print(&s2);
    printf("\n");

    hashmap_del(map, cast(a));

    printf("%d\n", hashmap_has(map, cast(a)));

    string_destroy_stk(&str);
    string_destroy_stk(&s2);
    hashmap_destroy(map);
    return 0;
}


// test push_move - string stored as value
int hashmap_test_2(void)
{
    hashmap* map = hashmap_create(sizeof(int), sizeof(String), NULL, NULL, NULL, str_copy,
                                  NULL, str_move, NULL, str_del);

    int a = 7;
    String* str = string_from_cstr("hello");
    hashmap_put(map, cast(a), 0, cast(str), 1);

    str = string_from_cstr("what is up");
    a += 2;
    hashmap_put(map, cast(a), 0, cast(str), 1);
    
    hashmap_print(map, int_print, str_print);

    
    hashmap_destroy(map);
    return 0;
}



