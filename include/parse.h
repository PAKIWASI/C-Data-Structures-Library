#pragma once


#include "hashmap.h"
#include "String.h"
#include "str_setup.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


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
    hashmap* map = hashmap_create(sizeof(String), sizeof(int), 
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
                    // Pass String by value (dereference the pointer)
                    if (hashmap_get(map, str_token, &count) == 0) {
                        count++;
                        hashmap_put(map, str_token, &count);
                    }
                    else {
                        count = 1;
                        hashmap_put(map, str_token, &count);
                    }
                    
                    // Always destroy the temporary string
                    string_destroy(str_token);
                    
                    total_words++;
                }
            }
            token = strtok(NULL, delim);
        }
    }
    

    // Print summary
    printf("\nTotal words processed: %zu\n", total_words);
    printf("Unique words: %zu\n\n", map->size);

    // Query example
    String* str = string_from_cstr("gay");
    int count;
    if (hashmap_get(map, str, &count) == 0) {
        printf("count of %s: %d", string_to_cstr(str), count);
    } else {
        printf("not found");
    }
    
    string_destroy(str);
    hashmap_destroy(map);    printf("\n");
    return fclose(f);
}


