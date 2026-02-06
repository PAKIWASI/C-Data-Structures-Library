#include "String.h"
#include "hashmap.h"
#include "helpers.h"
#include "str_setup.h"



// test push (copy) - string stored as value
int hashmap_test_1(void)
{
    hashmap* map = hashmap_create(sizeof(int), sizeof(String), NULL, NULL, NULL, str_copy,
                                  NULL, str_move, NULL, str_del);

    int a = 5;
    String str;
    string_create_stk(&str, "hello");

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
    string_append_cstr(s, " what is up");
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
    String* str = string_from_cstr("hello");
    hashmap_put_val_move(map, cast(a), (u8**)&str);

    str = string_from_cstr("what is up");
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

void map_put(hashmap* map, const char* k, const char* v)
{
    String* s1 = string_from_cstr(k);
    String* s2 = string_from_cstr(v);
    hashmap_put_move(map, (u8**)&s1, (u8**)&s2);
}

int hashmap_test_3(void)
{

    hashmap* map = hashmap_create(sizeof(String), sizeof(String), 
            murmurhash3_str, str_cmp, str_copy, str_copy, str_move, str_move, str_del, str_del);

    map_put(map, "what", "up");

    String s1;
    string_create_stk(&s1, "what");

    for (u32 i = 0; i < 10000; i++) {       // damn
        String* val = (String*)hashmap_get_ptr(map, cast(s1));
        string_append_cstr(val, "__hi");
        hashmap_put(map, castptr(val), castptr(val)); 
    }

    hashmap_print(map, str_print, str_print);

    string_destroy_stk(&s1);
    hashmap_destroy(map);
    return 0;
}

int hashmap_test_4(void)
{
    hashmap* map = hashmap_create(sizeof(String), sizeof(String), 
            murmurhash3_str, str_cmp, str_copy, str_copy, str_move, str_move, str_del, str_del);

    String* s1 = string_from_cstr("hel");

    String* s2 = string_from_cstr("-lo");

    hashmap_put(map, castptr(s1), castptr(s2));
    hashmap_print(map, str_print, str_print);

    String* s3 = string_from_string(s1);

    string_append_cstr(s2, "-lo");
    hashmap_put(map, castptr(s3), castptr(s2));
    hashmap_print(map, str_print, str_print);

    String s4;
    string_create_stk(&s4, "");
    string_reserve_char(&s4, 100, 'x');
    string_print(&s4);
    printf("\n%d\n", s4.svo);

    hashmap_put(map, castptr(s2), cast(s4));
    hashmap_print(map, str_print, str_print);

    string_destroy(s1);
    string_destroy(s2);
    string_destroy(s3);
    string_destroy_stk(&s4);
    hashmap_destroy(map);
    return 0;
}


