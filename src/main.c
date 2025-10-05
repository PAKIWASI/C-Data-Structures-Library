#include "String.h"
#include "hashmap.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
    ======================= HASHMAP USAGE WITH STRING* KEYS AND INT VALS =================================
*/


// MurmurHash3 for String* objects
size_t murmurhash3_string_ptr(const void* key, size_t size) {
    (void)size;
    
    String** str_ptr = (String**)key;
    String* str = *str_ptr;
    const char* data = string_to_cstr(str);
    size_t len = string_len(str);
    
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t seed = 0x9747b28c;
    
    uint32_t h1 = seed;
    
    // Body - process 4-byte chunks
    const uint32_t* blocks = (const uint32_t*)data;
    const size_t nblocks = len / 4;
    
    for (size_t i = 0; i < nblocks; i++) {
        uint32_t k1 = blocks[i];
        
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;
        
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1 * 5 + 0xe6546b64;
    }
    
    // Tail - handle remaining bytes
    const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);
    uint32_t k1 = 0;
    
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << 15) | (k1 >> 17);
                k1 *= c2;
                h1 ^= k1;
    }
    
    // Finalization
    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    
    return h1;
}

int string_ptr_compare(const void* a, const void* b, size_t size)
{
    (void)size;  // Unused
    
    // a and b are pointers to String* (i.e., String**)
    String* str_a = *(String**)a;
    String* str_b = *(String**)b;
    
    if (!str_a && !str_b) { return 0; }
    if (!str_a) { return -1; }
    if (!str_b) { return 1; }
    
    return string_compare(str_a, str_b);
}

hashmap* map;

void string_del(void* elm) {
    String** str_ptr = (String**)elm;
    if (str_ptr && *str_ptr) {
        string_destroy(*str_ptr);
    }
}

// FIXED: Print String* properly
void print_str(const void* elm) {
    String** str_ptr = (String**)elm;
    if (str_ptr && *str_ptr) {
        printf("%s", string_to_cstr(*str_ptr));
    }
}

void print_int(const void* elm) {
    printf("%d", *(int*)elm);
}

// FIXED: Pass address of String* pointer
void put(const char* key, int val) {
    String* str = string_from_cstr(key);
    hashmap_put(map, &str, &val);  // Pass &str, not str
}

// FIXED: Pass address and clean up temporary String
int get(const char* key) {
    String* str = string_from_cstr(key);
    int val = -1;  // Default value if not found
    hashmap_get(map, &str, &val);  // Pass &str, not str
    string_destroy(str);  // Clean up temporary string
    return val;
}

// FIXED: user_data should be void*, not const void*
void int_modify(void* val, const  void* user_data) {
    *(int*)val += *(int*)user_data;
}

// FIXED: key should be String*, not int
void modify(const char* key, int delta) {
    String* str = string_from_cstr(key);
    hashmap_modify(map, &str, int_modify, &delta);
    string_destroy(str);  // Clean up temporary string
}

int main(void)
{
    map = hashmap_create(
        sizeof(String*),           // Key is a pointer
        sizeof(int),               // Value is an int
        murmurhash3_string_ptr,    // Hash function
        string_del,                // Delete function for String* keys
        NULL,                      // No special delete for int values
        string_ptr_compare         // Compare function
    );
    
    put("hello", 5);
    put("world", 10);
    put("wasi", 111);
    put("ullah", 101);
    
    hashmap_print(map, print_str, print_int);
    
    printf("\nGetting 'hello': %d\n", get("hello"));
    
    printf("\nModifying 'world' by +50\n");
    modify("world", 50);
    
    printf("\nAfter modification:\n");
    hashmap_print(map, print_str, print_int);
    
    hashmap_destroy(map);
    return 0;
}

