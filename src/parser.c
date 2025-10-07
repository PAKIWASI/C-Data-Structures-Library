#include "parse.h"
#include "hashmap.h"
#include "String.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// ========== Hash function for String* ==========
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

// ========== Compare function for String* ==========
int string_ptr_compare(const void* a, const void* b, size_t size) {
    (void)size;
    
    String* str_a = *(String**)a;
    String* str_b = *(String**)b;
    
    if (!str_a && !str_b) { return 0; }
    if (!str_a) { return -1; }
    if (!str_b) { return 1; }
    
    return string_compare(str_a, str_b);
}

// ========== Delete function for String* ==========
void string_ptr_delete(void* str_ptr) {
    String** p = (String**)str_ptr;
    if (p && *p) {
        string_destroy(*p);
        *p = NULL;
    }
}


// Helper function to clean and normalize a word - removes numbers and handles contractions
char* clean_word(const char* token, char* output, size_t output_size) {
    if (!token || !output || output_size == 0) { return NULL; }
    
    size_t len = strlen(token);
    size_t out_idx = 0;
    bool has_letters = false;
    
    for (size_t i = 0; i < len && out_idx < output_size - 1; i++) {
        unsigned char c = (unsigned char)token[i];
        
        // Skip digits completely - remove numbers like 1023
        if (isdigit(c)) {
            continue;
        }
        
        // Handle apostrophes (both ASCII ' and UTF-8 curly quotes)
        // UTF-8 right single quote is E2 80 99
        if (c == '\'' || (c == 0xE2 && i + 2 < len && 
                          (unsigned char)token[i+1] == 0x80 && 
                          (unsigned char)token[i+2] == 0x99)) {
            
            // If it's a UTF-8 curly quote, skip the 3-byte sequence
            size_t next_pos = i + 1;
            if (c == 0xE2) {
                i += 2; // Skip the remaining bytes of UTF-8 sequence
                next_pos = i + 1;
            }
            
            // Check what comes after the apostrophe
            bool has_letter_after = false;
            bool is_possessive = false;
            
            for (size_t j = next_pos; j < len; j++) {
                unsigned char next = (unsigned char)token[j];
                if (isalpha(next)) {
                    has_letter_after = true;
                    // Check if it's possessive ('s or 's at end of word)
                    if (tolower(next) == 's') {
                        // Look ahead to see if there's anything after the 's'
                        bool has_more = false;
                        for (size_t k = j + 1; k < len; k++) {
                            if (isalpha((unsigned char)token[k])) {
                                has_more = true;
                                break;
                            }
                        }
                        if (!has_more) {
                            is_possessive = true;
                        }
                    }
                    break;
                }
                // Skip over more apostrophes/quotes
                if (next != '\'' && next != 0xE2) {
                    break;
                }
            }
            
            // Keep apostrophe if we have letters before AND after, but NOT if it's possessive
            if (has_letters && has_letter_after && !is_possessive) {
                output[out_idx++] = '\'';
            }
            continue;
        }
        
        // Convert to lowercase for case insensitivity
        if (isalpha(c)) {
            c = tolower(c);
            output[out_idx++] = c;
            has_letters = true;
        }
    }
    
    output[out_idx] = '\0';
    
    // Only return valid words that contain at least one letter
    return (out_idx > 0 && has_letters) ? output : NULL;
}


int parse(void)
{
    hashmap* map = hashmap_create(sizeof(String*), sizeof(int), 
                                 murmurhash3_string_ptr, string_ptr_delete, 
                                 NULL, string_ptr_compare);

    // Basic delimiters - we'll handle punctuation more carefully
    const char* delim = " \n\t\r";
    
    FILE* f = fopen("../shakespeare.txt", "re");
    if (!f) {
        printf("error opening file\n");
        hashmap_destroy(map);
        return -1;
    }

    char line[512];
    char cleaned[256];
    size_t total_words = 0;
    
    while (fgets(line, sizeof(line), f)) {
        char* token = strtok(line, delim);
        while (token) {
            if (strlen(token) > 0) {
                // Clean and normalize the word
                if (clean_word(token, cleaned, sizeof(cleaned))) {
                    String* str_token = string_from_cstr(cleaned);

                    //printf("%s\t", cleaned);
                    
                    int count;
                    if (hashmap_get(map, &str_token, &count) == 0) {
                        // Word exists, increment count
                        count++;
                        hashmap_put(map, &str_token, &count);
                        string_destroy(str_token); // Clean up temporary string
                    }
                    else {
                        // New word, insert with count 1
                        count = 1;
                        hashmap_put(map, &str_token, &count);
                        // Don't destroy str_token here - hashmap owns it now
                    }
                    
                    total_words++;
                }
            }
            token = strtok(NULL, delim);
        }
    }
    

    // Print summary
    printf("\nTotal words processed: %zu\n", total_words);
    printf("Unique words: %zu\n\n", map->size);

    int count;
    String* str = string_from_cstr("gay");
    if (hashmap_get(map, &str, &count) == 0) {
        printf("count of %s: %d",string_to_cstr(str), count);
    } else {
        printf("not found");
    }
    
    string_destroy(str);
    hashmap_destroy(map);
    printf("\n");
    return fclose(f);
}


