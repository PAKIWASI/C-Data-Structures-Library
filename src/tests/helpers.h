#pragma once

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

int str_cmp_val(const u8* a, const u8* b, u32 size) 
{
    (void)size;
    return string_compare((String*)a, (String*)b); 
}

int str_cmp_ptr(const u8* a, const u8* b, u32 size) 
{
    (void)size;
    return string_compare(*(String**)a, *(String**)b); 
}


void int_print(const u8* elm) {
    printf("%d", *(int*)elm);
}



