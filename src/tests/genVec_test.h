#include "String.h"
#include <string.h>




// === test vec of string (sizeof(String)) ===
//============================================

// container stores String by value (40 bytes), not ptr
void str_copy(u8* dest, const u8* src)
{
    String* d = (String*)dest;  // malloced, not initalized container (garbage value)
    String* s = (String*)src;

    // copy all field values
    memcpy(d, s, sizeof(String));   // both point to same data

    // allocate space for data
    u32 n = s->buffer.size * s->buffer.data_size;
    d->buffer.data = malloc(n);
    
    // copy data
    memcpy(d->buffer.data, s->buffer.data, n);
}

// in case of String by val, buffer is malloced (but random)
// we copy buffer entirely (all fields)
// src must be heap allocated
void str_move(u8* dest, u8** src)
{
    memcpy(dest, *src, sizeof(String));

    free(*src); 
    *src = NULL;
}


// del buffer, not ptr
void str_del(u8* elm)
{
    string_destroy_stk((String*)elm);
}

void str_print(const u8* elm)
{
    string_print((const String*)elm);
}


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


// === test vec of string* (sizeof(String*)) ===
//==============================================

// when we try to copy elements from one vec to another, then src is also string**
// so we take input as string**, always
// so when we create stack str, then we have to assign ptr to it and pass address of that


// vec stores pointers to strings, we get a ptr to the element, which is a ptr to string
void str_copy_ptr(u8* dest, const u8* src)
{
    String* s = *(String**)src;       // double ptr to str (input) 

    // allocate memory for heap string, point d to it
    String* d = malloc(sizeof(String));

    // copy all the fields
    memcpy(d, s, sizeof(String));

    // allocate memroy for new data
    u32 n = s->buffer.size * s->buffer.data_size;
    d->buffer.data = malloc(n);

    // copy all elements
    memcpy(d->buffer.data, s->buffer.data, n);

    *(String**)dest = d;    // dest is double ptr to str
}


// dest & src are double ptrs
void str_move_ptr(u8* dest, u8** src)
{
    // give address of String to dest 
    *(String**)dest = *(String**)src;

    *src = NULL;
}

// elm is
void str_del_ptr(u8* elm)
{
    String* e = *(String**)elm;
    // destroy completely (elm fully on heap)
    string_destroy(e);
    // vec container just a ptr (vec owns it)
}

void str_print_ptr(const u8* elm)
{
    string_print(*(const String**)elm);
}


// test push (copy) 
int genVec_test_3(void)
{
    genVec* vec = genVec_init(10, sizeof(String*), str_copy_ptr, str_move_ptr, str_del_ptr);

    String str;
    u8* p = (u8*)&str;
    string_create_stk((String*)p, "hello");
    genVec_push(vec, (const u8*)&p); //  address of pointer to str (double ptr)

    string_print((String*)p);
    string_destroy_stk((String*)p);

    genVec v2;
    genVec_copy(&v2, vec);

    genVec_print(vec, str_print_ptr);
    genVec_print(&v2, str_print_ptr);

    genVec_destroy(vec);
    genVec_destroy_stk(&v2);

    return 0;
}

// test push_move 
int genVec_test_4(void)
{
    genVec* vec = genVec_init(10, sizeof(String*), str_copy_ptr, str_move_ptr, str_del_ptr);

    String* str = string_from_cstr("hello");
    genVec_push_move(vec, (u8**)&str); // take String** for move

    genVec* v2 = malloc(sizeof(genVec));
    genVec_move(v2, &vec);

    
    genVec_print(v2, str_print_ptr);
    genVec_destroy(v2);

    return 0;
}
    

