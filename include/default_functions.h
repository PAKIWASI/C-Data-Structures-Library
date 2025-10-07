#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define LOAD_FACTOR_GROW 0.70
#define LOAD_FACTOR_SHRINK 0.20  
#define HASHMAP_INIT_CAPACITY 17  //prime no (index = hash % capacity)

typedef uint8_t u8;

/*
====================DEFAULT FUNCTIONS====================
*/
// 32-bit FNV-1a (default hash)
static size_t fnv1a_hash(const u8* bytes, size_t size) {
    uint32_t hash = 2166136261U;  // FNV offset basis

    for (size_t i = 0; i < size; i++) {
        hash ^= bytes[i];   // XOR with current byte
        hash *= 16777619U;  // Multiply by FNV prime
    }

    return hash;
}

// default delete function (works for basic data types)
static void default_delete(u8* elm) {
    if (elm) {
        free(elm);
    }
}

// Default compare function
static int default_compare(const u8* a, const u8* b, size_t size) 
{
    return memcmp(a, b, size);
}


