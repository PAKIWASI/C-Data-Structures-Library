#pragma once

#include <stddef.h>



typedef enum {
    EMPTY = 0,
    FILLED = 1,
    TOMBSTONE = 2
} STATE;


typedef void (*delete_fn)(void* key); //optional (for string*, arrays, structs as keys, vals)
typedef size_t (*custom_hash_fn)(const void* key, size_t size);     // optional (for a specific usecase)
typedef int (*compare_fn)(const void* a, const void* b, size_t size);   // optional: for compare operations

