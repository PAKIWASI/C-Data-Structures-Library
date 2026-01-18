#pragma once

#include "common.h"
#include <string.h>


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



#define LOAD_FACTOR_GROW 0.70
#define LOAD_FACTOR_SHRINK 0.20  
#define HASHMAP_INIT_CAPACITY 17  //prime no (index = hash % capacity)


/*
====================DEFAULT FUNCTIONS====================
*/
// 32-bit FNV-1a (default hash)
static u32 fnv1a_hash(const u8* bytes, u32 size) {
    u32 hash = 2166136261U;  // FNV offset basis

    for (u32 i = 0; i < size; i++) {
        hash ^= bytes[i];   // XOR with current byte
        hash *= 16777619U;  // Multiply by FNV prime
    }

    return hash;
}


// Default compare function
static int default_compare(const u8* a, const u8* b, u32 size) 
{
    return memcmp(a, b, size);
}

static const u32 PRIMES[] = {
    17, 37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, 
    21911, 43853, 87719, 175447, 350899, 701819, 1403641, 
    2807303, 5614657, 11229331, 22458671, 44917381, 89834777
};

static const u32 PRIMES_COUNT = sizeof(PRIMES) / sizeof(PRIMES[0]);

// Find the next prime number larger than current
static u32 next_prime(u32 current) {
    for (u32 i = 0; i < PRIMES_COUNT; i++) {
        if (PRIMES[i] > current) {
            return PRIMES[i];
        }
    }
    
    // If we've exceeded our prime table, fall back to approximate prime
    // Using formula: next ≈ current * 2 + 1 (often prime or close to it)
    printf("Warning: exceeded prime table, using approximation\n");
    return (current * 2) + 1;
}

// Find the previous prime number smaller than current
static u32 prev_prime(u32 current) {
    // Search backwards through prime table
    for (u32 i = PRIMES_COUNT - 1; i >= 0; i--) {
        if (PRIMES[i] < current) {
            return PRIMES[i];
        }
    }
    
    // Should never happen if HASHMAP_INIT_CAPACITY is in our table
    printf("Warning: no smaller prime found\n");
    return HASHMAP_INIT_CAPACITY;
}




