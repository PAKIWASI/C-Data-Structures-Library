#include "String.h"
#include "gen_vector.h"
#include "str_setup.h"


void str_print(const u8 *elm)
{
    string_print((String*)elm);
}


void string_copy(u8* copy, const u8* src)
{
    String* s = (String*) src;
    String* c = (String*) copy;

    string_append_cstr(c, string_to_cstr(s)); 
}


int genVec_test_1(void)
{
    genVec* vec = genVec_init(10, sizeof(String), string_copy, string_custom_delete);

    String* str = string_from_cstr("hello");
    genVec_push(vec, castptr(str));
    string_destroy(str);

    genVec_print(vec, str_print);

    genVec_destroy(vec);
    return 0;
}
