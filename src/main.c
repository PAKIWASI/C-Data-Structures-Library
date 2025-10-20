#include "hashmap.h"
#include "hashset.h"
#include "helper_functions.h"
#include "str_setup.h"


int main(void)
{
    map = hashmap_create(sizeof(String), sizeof(int), murmurhash3_string, string_custom_delete, NULL, NULL);

    map_put_strToInt("hello", 1);
    map_put_strToInt("world", 10);
    map_put_strToInt("!!!", 3);

    hashmap_print(map, str_print, int_print);

    hashmap_destroy(map);

    set = hashset_create(sizeof(int), NULL, NULL, NULL);

    set_insert_int(1);
    set_insert_int(2);
    set_insert_int(3);
    set_insert_int(4);

    hashset_print(set, int_print);

    if (set_has_int(100)) {
        printf("set has 4\n");
    } else {
        printf("set does not have 4\n");
    }

    hashset_destroy(set);
    return 0;
}
