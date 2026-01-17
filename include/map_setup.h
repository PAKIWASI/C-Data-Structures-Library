#pragma once

#include "common.h"


typedef enum {
    EMPTY = 0,
    FILLED = 1,
    TOMBSTONE = 2
} STATE;


typedef void (*copy_fn)(u8* dest, const u8* src);
typedef void (*move_fn)(u8* dest, u8** src);
typedef void (*delete_fn)(u8* key); 

typedef u32 (*custom_hash_fn)(const u8* key, u32 size);     
typedef int (*compare_fn)(const u8* a, const u8* b, u32 size);


