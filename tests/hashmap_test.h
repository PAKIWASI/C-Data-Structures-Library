#include "common.h"
#include "hashmap.h"
#include "helpers.h"



// test push (copy) - string stored as value
int hashmap_test_1(void)
{
    hashmap* map = hashmap_create(sizeof(int), sizeof(String), NULL, NULL, NULL, str_copy,
                                  NULL, str_move, NULL, str_del);

    int a = 5;
    String str;
    string_create_stk(&str, "hello", 5);

    hashmap_put(map, cast(a), cast(str));
    a++;
    hashmap_put(map, cast(a), cast(str));
    a++;
    hashmap_put(map, cast(a), cast(str));
    a++;
    hashmap_put(map, cast(a), cast(str));
    a++;
    hashmap_put(map, cast(a), cast(str));
    a++;
    hashmap_put(map, cast(a), cast(str));
    a++;
    hashmap_put(map, cast(a), cast(str));

    hashmap_print(map, int_print, str_print);

    String* s = (String*)hashmap_get_ptr(map, cast(a));
    STR_APPEND_CSTR(s, " what is up");
    string_print(s);
    printf("\n");
    s = NULL;

    hashmap_print(map, int_print, str_print);
    
    String s2 = {0};
    hashmap_get(map, cast(a), cast(s2));
    string_print(&s2);
    printf("\n");

    hashmap_del(map, cast(a), NULL);

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
    String* str = string_from_cstr("hello", 5);
    hashmap_put_val_move(map, cast(a), (u8**)&str);

    str = string_from_cstr("what is up", 10);
    a += 2;
    hashmap_put_val_move(map, cast(a), (u8**)&str);
    
    hashmap_print(map, int_print, str_print);

    String s;
    hashmap_del(map, cast(a), cast(s)); // in this case s is output so (u8*)
    string_print(&s);
    printf("\n");

    hashmap_print(map, int_print, str_print);

    printf("%d\n", hashmap_size(map));
    
    string_destroy_stk(&s);
    hashmap_destroy(map);
    return 0;
}



