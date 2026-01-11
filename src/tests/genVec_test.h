#include "String.h"
#include "common.h"
#include "gen_vector.h"
#include "str_setup.h"


void str_print(const u8 *elm)
{
    string_print((String*)elm);
}


void string_copy(u8* copy, const u8* src)
{
    String* c = (String*) copy;
    String* s = (String*) src;

    genVec_copy(&c->buffer, &s->buffer);
}



int genVec_test_1(void)
{
    genVec* vec = genVec_init(10, sizeof(String), string_copy, string_custom_delete);

    String* str = string_from_cstr("hello");
    genVec_push_move(vec, (u8**)(&str));

    genVec_print(vec, str_print);

    genVec_destroy(vec);
    return 0;
}
