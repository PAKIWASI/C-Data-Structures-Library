#pragma once

#include "common.h"


typedef enum {
    EMPTY = 0,
    FILLED = 1,
    TOMBSTONE = 2
} STATE;


typedef void (*delete_fn)(u8* key); //optional (for string*, arrays, structs as keys, vals)
typedef u32 (*custom_hash_fn)(const u8* key, u32 size);     // optional (for a specific usecase)
typedef int (*compare_fn)(const u8* a, const u8* b, u32 size);   // optional: for compare operations
