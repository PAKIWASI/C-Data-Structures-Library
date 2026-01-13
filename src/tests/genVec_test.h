#include "String.h"
#include "gen_vector.h"


void str_copy(u8* copy, const u8* src)
{
    String* c = (String*)copy;
    String* s = (String*)src;

    //string_append_string(c, s);
}

// del buffer, not ptr
void str_del(u8* elm)
{
    string_destroy_fromstk((String*)elm);
}

void str_print(const u8* elm)
{
    string_print((const String*)elm);
}


int genVec_test_1(void)
{
    String str;
    string_create_onstk(&str, "");
    genVec* vec = genVec_init_val(10, cast(str), sizeof(String), str_copy, str_del);


    genVec_print(vec, str_print);


    genVec_destroy(vec);

    return (vec == NULL) ? 0 : 1;

    return 0;
}
