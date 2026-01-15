#include "String.h"
#include "common.h"
#include "gen_vector.h"
#include <stdio.h>
#include <string.h>



// container stores String by value (40 bytes), not ptr
void str_copy(u8* copy, const u8* src)
{
    String* c = (String*)copy;  // not initalized container (garbage value)
    String* s = (String*)src;

    // copy all field values
    memcpy(c, s, sizeof(String));   // both point to same data

    // allocate space for data
    u32 n = s->buffer.size * s->buffer.data_size;
    c->buffer.data = malloc(n);
    
    // copy data
    memcpy(c->buffer.data, s->buffer.data, n);
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




// === test vec of string (sizeof(String))

// test push (copy)
int genVec_test_1(void)
{
    genVec* vec = genVec_init(10, sizeof(String), str_copy, str_del);

    String* str = string_from_cstr("hello");

    genVec_push(vec, castptr(str));
    genVec_push(vec, castptr(str));

    string_append_cstr(str, " what is up");

    genVec_push(vec, castptr(str));
    genVec_push(vec, castptr(str));

    string_print(str);
    string_destroy(str);

    genVec_print(vec,  str_print);
    genVec_destroy(vec);

    return 0;
}

// test push_move (transfer ownership)
int genVec_test_2(void)
{
    genVec* vec = genVec_init(10, sizeof(String), str_copy, str_del);

    String* str = string_from_cstr("hello");

    string_print(str);
    printf("\n");

    genVec_push_move(vec, (u8**)&str);


    genVec_print(vec,  str_print);
    genVec_destroy(vec);

    return 0;
}



