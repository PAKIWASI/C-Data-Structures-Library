#include "String.h"
#include "common.h"
#include "gen_vector.h"
#include <string.h>


// container stores String by value (40 bytes), not ptr
void str_copy(u8* dest, const u8* src)
{
    String* c = (String*)dest;  // malloced, not initalized container (garbage value)
    String* s = (String*)src;

    // copy all field values
    memcpy(c, s, sizeof(String));   // both point to same data

    // allocate space for data
    u32 n = s->buffer.size * s->buffer.data_size;
    c->buffer.data = malloc(n);
    
    // copy data
    memcpy(c->buffer.data, s->buffer.data, n);
}

// in case of String by val, buffer is malloced (but random)
// we copy buffer entirely (all fields)
void str_move(u8* dest, u8** src)
{
    memcpy(dest, *src, sizeof(String));

    String* s = *(String**)src;

    s->buffer.data = NULL;
    string_destroy(s);
    *src = NULL;
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
    genVec* vec = genVec_init(10, sizeof(String), str_copy, str_move, str_del);

    String* str = string_from_cstr("hello");
    genVec_push(vec, castptr(str));
    genVec_push(vec, castptr(str));

    string_append_cstr(str, " what is up");
    genVec_push(vec, castptr(str));
    genVec_push(vec, castptr(str));

    string_print(str);
    string_destroy(str);

    // Initialize v2 before copying
    genVec v2;
    genVec_copy(&v2, vec);

    genVec_print(vec, str_print);
    genVec_print(&v2, str_print);
    
    genVec_destroy(vec);
    genVec_destroy_stk(&v2);

    return 0;
}

// test push_move (transfer ownership)
int genVec_test_2(void)
{
    genVec* vec = genVec_init(10, sizeof(String), str_copy, str_move, str_del);

    String* str = string_from_cstr("hello");
    string_print(str);
    printf("\n");

    genVec_push_move(vec, (u8**)&str);

    // Move heap vector to stack variable
    genVec v2;
    genVec_move(&v2, &vec);  // vec is now NULL

    genVec_print(&v2, str_print);
    genVec_destroy_stk(&v2);

    return 0;
}


// === test vec of string* (sizeof(String*))


// test push (copy) 
int genVec_test_3(void)
{

    return 0;
}


    

/*      COPY vs MOVE - PUSH vs PUSH_MOVE
 * If we are copying by value, then sizeof(String) is easier (copy_fn for push) 
 * If we are transfering, then sizeof(String*) is easier (move_fn for push_move)
 *
 * SIZEOF(STRING):
 * copy is easy as we just copy buffer, then copy data for size n
 * for move, space of dest is already inited. we memcpy src to dest, we get all fields. then set src's data to null and call destroy
 * genVec_copy
 * SIZEOF(STRING*):
 *
*/


