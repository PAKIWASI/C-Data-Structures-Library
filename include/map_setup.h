#pragma once

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;


typedef enum {
    EMPTY = 0,
    FILLED = 1,
    TOMBSTONE = 2
} STATE;


typedef void (*delete_fn)(u8* key); //optional (for string*, arrays, structs as keys, vals)
typedef size_t (*custom_hash_fn)(const u8* key, size_t size);     // optional (for a specific usecase)
typedef int (*compare_fn)(const u8* a, const u8* b, size_t size);   // optional: for compare operations
