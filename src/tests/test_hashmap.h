
#include "hashmap.h"
#include "helper_functions.h"


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

    hashmap_destroy(map);
    return 0;
}
