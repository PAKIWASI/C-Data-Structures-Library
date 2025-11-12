
#include "String.h"
#include "gen_vector.h"
#include "hashmap.h"
#include "helper_functions.h"
#include "str_setup.h"


int test_hashmap_1(void)
{
    hashmap* map  = hashmap_create(sizeof(int), sizeof(int), NULL, NULL, NULL, NULL);


    map_put_intToInt(map, 1, 1);
    map_put_intToInt(map, 2, 1);
    map_put_intToInt(map, 3, 1);
    map_put_intToInt(map, 4, 1);
    map_put_intToInt(map, 5, 1);
    map_put_intToInt(map, 6, 1);
    map_put_intToInt(map, 7, 1);
    map_put_intToInt(map, 8, 1);
    map_put_intToInt(map, 9, 1);
    map_put_intToInt(map, 0, 1);
    map_put_intToInt(map, 11, 1);
    map_put_intToInt(map, 12, 1);
    map_put_intToInt(map, 14, 1);
    map_put_intToInt(map, 15, 1);
    map_put_intToInt(map, 16, 1);
    map_put_intToInt(map, 17, 1);
    map_put_intToInt(map, 18, 1);
    map_put_intToInt(map, 19, 1);
    map_put_intToInt(map, 10, 1);
    map_put_intToInt(map, 20, 1);
    map_put_intToInt(map, 21, 1);
    map_put_intToInt(map, 22, 1);
    map_put_intToInt(map, 23, 1);
    map_put_intToInt(map, 24, 1);
    map_put_intToInt(map, 25, 1);
    map_put_intToInt(map, 26, 1);
    map_put_intToInt(map, 27, 1);
    map_put_intToInt(map, 28, 1);

    hashmap_print(map, int_print, int_print);

    map_del_intToInt(map, 4);
    map_del_intToInt(map, 4);
    map_del_intToInt(map, 7);
    map_del_intToInt(map, 10);
    map_del_intToInt(map, 20);
    map_del_intToInt(map, 21);
    map_del_intToInt(map, 22);
    map_del_intToInt(map, 2);

    map_put_intToInt(map, 20, 10);
    map_put_intToInt(map, 1, 100);

    hashmap_print(map, int_print, int_print);

    int a = 0;
    *hashmap_get_ptr(map, cast(a)) += 200;

    hashmap_print(map, int_print, int_print);

    printf("%d\n", map_has_intToInt(map, 5));
    printf("%d\n", map_has_intToInt(map, 20));

    hashmap_destroy(map);
    return 0;
}


// TEST 2: STR -> STR
int test_hashmap_2(void)
{
    hashmap* map = hashmap_create(sizeof(String), sizeof(String), murmurhash3_string, string_custom_delete, string_custom_delete, string_custom_compare);
    
    map_put_strToStr(map, "hello", "world");
    map_put_strToStr(map, "hello", "world2");
    map_put_strToStr(map, "hello ", "world3");
    map_put_strToStr(map, "helllo", "world4");
    map_put_strToStr(map, " hello", "world5");

    map_put_strToStr(map, "wasi", "ullah");
    map_put_strToStr(map, "ali", "abdullah");
    map_put_strToStr(map, "M", "ahmed");

    hashmap_print(map, str_print, str_print);

    map_del_strToStr(map, " hello");
    map_del_strToStr(map, "hello ");

    hashmap_print(map, str_print, str_print);

    printf("%d\n", map_has_strToStr(map, "helllo"));
    printf("%d\n", map_has_strToStr(map, " hello"));

    string_print((const String*)map_get_ptr_strtoStr(map, "ali"));
    printf("\n");

    string_append_cstr((String*)map_get_ptr_strtoStr(map, "ali"), " abbasi");

    hashmap_print(map, str_print, str_print);

    hashmap_destroy(map);
    return 0;
}

static inline void vec_custom_del(u8* elm) { // we create on stk 
    genVec_destroy_stk((genVec*)elm);
}

void map_put_intToVec(hashmap* map, int key, genVec* vec) {
    hashmap_put(map, cast(key), (u8*)vec);
}

void vec_print_int(const u8* elm) {
    genVec_print((const genVec*)elm, int_print);
}

// TEST 3: INT -> VEC
int test_hashmap_3(void)
{
    hashmap* map = hashmap_create(sizeof(int), sizeof(genVec), NULL, NULL, vec_custom_del, NULL);
    
    for (int i = 0; i < 10; i++) { // keys
        genVec vec;
        genVec_init_stk(5, sizeof(int), NULL, &vec);
        vec_push_ints(&vec, 1,2,3,4,5);

        map_put_intToVec(map, i, &vec);
    }

    hashmap_print(map, int_print, vec_print_int);

    int a = 1;
    vec_push_ints((genVec*)hashmap_get_ptr(map, cast(a)), 10, 11, 12, 13);

    hashmap_print(map, int_print, vec_print_int);

    hashmap_destroy(map);
    return 0;
}
