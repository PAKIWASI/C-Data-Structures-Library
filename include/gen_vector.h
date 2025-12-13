#pragma once

#include <stddef.h>
#include <stdint.h>


typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;

//user provided
typedef void (*genVec_print_fn)(const u8* elm);
typedef u8   (*genVec_compare_fn)(const u8* a, const u8* b);
typedef void (*genVec_delete_fn)(u8* elm);
typedef void (*genVec_copy_fn)(u8* copy, const u8* src);


typedef struct {
    u8*               data;
    u32               size;
    u32               capacity;
    u16               data_size;
    genVec_delete_fn  del_fn;
    genVec_copy_fn    copy_fn;

} genVec;


//memory management

        // init empty vector of size = n, del_fn needed if data is complex(pointer to some other data) otherwise NULL
genVec* genVec_init(u32 n, u16 data_size, genVec_delete_fn del_fn);
        // init a vector on stack with the data on the heap
void    genVec_init_stk(u32 n, u16 data_size, genVec_delete_fn del_fn, genVec* vec);
        // init empty vector of size = n, each elm with val = val, del_fn needed if data is complex(pointer to some other data) otherwise NULL
genVec* genVec_init_val(u32 n, const u8* val, u16 data_size, genVec_delete_fn del_fn);
        // destroy vec, using previously provided del_fn
void    genVec_destroy(genVec* vec);
        // destroy the stack version of the vector
void    genVec_destroy_stk(genVec* vec);
        // delete all elm in vector, shrink to 0
void    genVec_clear(genVec* vec);
        // grow vector to new_capacity, keeping previous elms
void    genVec_reserve(genVec* vec, u32 new_capacity);
        // grow vector to new_capacrity and size with new vals = val
        // SHOULD ONLY BE USED FOR BASIC DATATYPES (POINTERS ARE SHALLOW COPIED)
void    genVec_reserve_val(genVec* vec, u32 new_capacrity, const u8* val);

//operations

        // push to the back of the vec (COPIES DATA- Shallow copy)
void genVec_push(genVec* vec, const u8* data);
        // pop from the back of the vec, OPTIONAL: if popped provided, copy data to it
void genVec_pop(genVec* vec, u8* popped);
        // get a copy of the elm at index i in buffer out
void genVec_get(const genVec* vec, u32 i, u8* out);
        // get a pointer to the data at index i
const u8* genVec_get_ptr(const genVec* vec, u32 i);
        // repace elm at index i with data, deleting old elm
void genVec_replace(genVec* vec, u32 i, const u8* data);
        // insert at data index i, shifting elm to the right
void genVec_insert(genVec* vec, u32 i, const u8* data);
        // insert multiple elements of val = data at once, starting at pos i, with num_data no of elms 
void genVec_insert_multi(genVec* vec, u32 i, const u8* data, u32 num_data);
        // remove elm at index i, deleting the elm and shifting arr to left
void genVec_remove(genVec* vec, u32 i);
        // remove elms in the range of l, r (included)
void genVec_remove_range(genVec* vec, u32 l, u32 r);
        // get the 0th index elm in buffer out
u8*  genVec_front(const genVec* vec);
        // get the elm at the back of the vec in buffer out
u8*  genVec_back(const genVec* vec);

//utility

        // print the elms to stdout, using a print function fn
void genVec_print(const genVec* vec, genVec_print_fn fn);

void genVec_copy(genVec* dest, const genVec* src, genVec_copy_fn copy_fn);

        // get no of elms in the vec
static inline u32 genVec_size(const genVec* vec) {
    return vec ? vec->size : 0;
}

        // get total capacity of the vec
static inline u32 genVec_capacity(const genVec* vec) {
    return vec ? vec->capacity : 0;
}

        // boolean to query empty
static inline u8 genVec_empty(const genVec* vec) {
    return vec ? vec->size == 0 : 0;
}

