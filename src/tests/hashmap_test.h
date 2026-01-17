#include "String.h"
#include "common.h"
#include "hashmap.h"
#include "helpers.h"
#include <string.h>


int hashmap_test_1(void)
{
    hashmap* map = hashmap_create(sizeof(int), sizeof(String), NULL, str_cmp_val, NULL, str_copy,
                                  NULL, str_move, NULL, str_del);

    int a = 5;
    String str;
    string_create_stk(&str, "hello");

    hashmap_put(map, cast(a), cast(str));

    hashmap_print(map, int_print, str_print);

    hashmap_destroy(map);
    
    return 0;
}
