#include "String.h"
#include "genVec_test.h"
#include "hashmap.h"


#define PUT_INT_STR(map, key, val)             \
    do {                                       \
        int     k = key;                       \
        String* v = string_from_cstr(val);     \
        hashmap_put(map, cast(k), castptr(v)); \
    } while (0)


void int_print(const u8* elm)
{
    printf("%d", *(int*)elm);
}

int hashmap_test_1(void)
{
    hashmap* map = hashmap_create(sizeof(int), sizeof(String*), NULL, NULL, NULL, str_del_ptr);

    PUT_INT_STR(map, 1, "hello");

    hashmap_print(map, int_print, str_print_ptr);

    hashmap_destroy(map);
    return 0;
}
