#pragma once

#include "String.h"
#include "gen_vector.h"
#include <stdint.h>


// ========== Hash function for String* ==========
u32 murmurhash3_string(const u8* key, u32 size) 
{
    (void)size;
    
    String* str= (String*)key;
    const char* data = string_to_cstr(str);
    u32 len = string_len(str);
    
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;
    const uint32_t seed = 0x9747b28c;
    
    uint32_t h1 = seed;
    
    // Body - process 4-byte chunks
    const uint32_t* blocks = (const uint32_t*)data;
    const u32 nblocks = len / 4;
    
    for (u32 i = 0; i < nblocks; i++) {
        uint32_t k1 = blocks[i];
        
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;
        
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = (h1 * 5) + 0xe6546b64;
    }
    
    // Tail - handle remaining bytes
    const uint8_t* tail = (const uint8_t*)(data + ((size_t)nblocks * 4));
    uint32_t k1 = 0;
    
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << 15) | (k1 >> 17);
                k1 *= c2;
                h1 ^= k1;
        default:
           break; 
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

// ========== Compare function for String* ==========
int string_custom_compare(const u8* a, const u8* b, u32 size) 
{
    (void)size;
    return string_compare((String*)a, (String*)b); 
}

// ========== Delete function for String* ==========
void string_custom_delete(u8* str_ptr) {
    String*  str = (String*)str_ptr;
    if (str && str->buffer) {
        //only destroy buffer and not String as hashmap stores String not String*
        genVec_destroy(str->buffer);
        str->buffer = NULL;
    }
}


