#pragma once

#include <stdint.h>
#include <stdio.h>
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


// Default compare function
static int default_compare(const u8* a, const u8* b, size_t size) 
{
    return memcmp(a, b, size);
}

static const size_t PRIMES[] = {
    17, 37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, 
    21911, 43853, 87719, 175447, 350899, 701819, 1403641, 
    2807303, 5614657, 11229331, 22458671, 44917381, 89834777
};

static const size_t PRIMES_COUNT = sizeof(PRIMES) / sizeof(PRIMES[0]);

// Find the next prime number larger than current
static size_t next_prime(size_t current) {
    for (size_t i = 0; i < PRIMES_COUNT; i++) {
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
static size_t prev_prime(size_t current) {
    // Search backwards through prime table
    for (size_t i = PRIMES_COUNT - 1; i >= 0; i--) {
        if (PRIMES[i] < current) {
            return PRIMES[i];
        }
    }
    
    // Should never happen if HASHMAP_INIT_CAPACITY is in our table
    printf("Warning: no smaller prime found\n");
    return HASHMAP_INIT_CAPACITY;
}


