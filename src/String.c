#include "String.h"
#include "gen_vector.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



// Private helper to ensure null termination
static void ensure_null_terminated(String* str) {
    if (!str || !str->buffer) { return; }

    const char null_term = '\0';
    
    if (str->buffer->size == 0) {
        // empty buffer, just add null terminator
        genVec_push(str->buffer, &null_term);
    } else {
        // check if last character is already null terminator
        char last_char;
        genVec_get(str->buffer, str->buffer->size - 1, &last_char);
        
        if (last_char != '\0') {
            // Last character is not null terminator, so append one
            genVec_push(str->buffer, &null_term);
        }
    }
}

String* string_create(void) {
    String* str = malloc(sizeof(String));
    if (!str) { 
        printf("str create: malloc failed\n");
        return NULL; 
    }
    
    // we get a pointer to buffer
    str->buffer = genVec_init(0, sizeof(char), NULL);
    if (!str->buffer) {
        printf("str create: genVec_init failed\n");
        free(str);
        return NULL;
    }
    
    ensure_null_terminated(str);
    return str;
}

String* string_from_cstr(const char* cstr) {
    if (!cstr) { 
        printf("str from cstr: cstr is null\n");
        return NULL; 
    }
    
    String* str = string_create();
    if (!str) { 
        printf("str from cstr: string_create failed\n");
        return NULL; 
    }
    
    string_append_cstr(str, cstr);
    return str;
}

String* string_from_string(const String* other) {
    if (!other) { 
        printf("string from string: other is null\n");
        return NULL; 
    }
    return string_from_cstr(string_to_cstr(other));
}

void string_reserve(String* str, size_t capacity) 
{
    if (!str || !str->buffer) { return; }
    
    // Add 1 for null terminator
    capacity++;
    
    // Use genVec_reserve to pre-allocate
    genVec_reserve(str->buffer, capacity);
    
    // Ensure null termination is preserved
    ensure_null_terminated(str);
}

void string_destroy(String* str) {
    if (str) {
        genVec_destroy(str->buffer);
        free(str);
    }
}

size_t string_len(const String* str) {
    if (!str || !str->buffer) { 
        printf("str len: str is null or buffer is null\n");
        return 0; 
    }
    // Length is size - 1 (excluding null terminator)
    return str->buffer->size > 0 ? str->buffer->size - 1 : 0;
}

int string_empty(const String* str) {
    return string_len(str) == 0;
}

const char* string_to_cstr(const String* str) {
    if (!str || !str->buffer || str->buffer->size == 0) { 
        printf("str to cstr: null or empty\n");
        return ""; 
    }
    return (const char*)str->buffer->data; // last char is '\0' so return till that
}

void string_append_cstr(String* str, const char* cstr) {
    if (!str || !cstr || !str->buffer) { 
        printf("str append cstr: invalid parameters\n");
        return; 
    }
    
    size_t cstr_len = strlen(cstr);
    if (cstr_len == 0) { return; }// Nothing to append

    // Calculate where to start writing (before null terminator (if it exists))
    size_t write_pos = str->buffer->size;
    if (write_pos > 0) {
        char last_char;
        genVec_get(str->buffer, write_pos - 1, &last_char);
        if (last_char == '\0') {
            write_pos--; // Overwrite the null terminator
            str->buffer->size--; // Temporarily reduce size
        }
    }
    
    // Append all characters at once would be more efficient,
    // but since we're using the generic vector, we'll append one by one
    for (size_t i = 0; i < cstr_len; i++) {
        genVec_push(str->buffer, &cstr[i]);
    }
    
    // Ensure null termination
   ensure_null_terminated(str); 
}

void string_append_string(String* str, const String* other) {
    if (!str || !other) { 
        printf("str append str: parameters null\n");
        return; 
    }
    string_append_cstr(str, string_to_cstr(other));
}

void string_append_char(String* str, char c) {
    if (!str) { 
        printf("str append char: str null\n");
        return; 
    }
    
    // Remove null terminator temporarily
    if (str->buffer->size > 0) {
        genVec_pop(str->buffer, NULL);
    }
    
    genVec_push(str->buffer, &c);
    ensure_null_terminated(str);
}


void string_remove_char(String* str, size_t i) {
    if (!str || !str->buffer) { 
        printf("str remove char: str or buffer null\n");
        return; 
    }
    
    size_t len = string_len(str);
    if (i >= len) { 
        printf("str remove char: index %zu out of bounds (len=%zu)\n", i, len);
        return; 
    }
    
    // Remove the character at index i
    genVec_remove(str->buffer, i);
    
    // The null terminator should still be there, but im scared
    if (str->buffer->size == 0 || 
        (str->buffer->size > 0 && ((char*)str->buffer->data)[str->buffer->size - 1] != '\0')) {
        ensure_null_terminated(str);
    }
}

void string_clear(String* str) {
    if (!str) { 
        printf("str clear: str null\n");
        return; 
    }
    
    genVec_clear(str->buffer);
    ensure_null_terminated(str);
}

char string_at(const String* str, size_t i) {
    if (!str || i >= string_len(str)) { 
        printf("str at: str null or i out of bounds\n");
        return '\0'; 
    }
    
    char c;
    genVec_get(str->buffer, i, &c);
    return c;
}

void string_set_char(String* str, size_t i, char c) {
    if (!str || i >= string_len(str)) { 
        printf("str set char: str null or i out of bounds\n");
        return; 
    }
    genVec_replace(str->buffer, i, &c);
}

int string_compare(const String* str1, const String* str2) {
    if (!str1 || !str2) { 
        printf("str comp: parameters null\n");
        return -1; 
    }
    return strcmp(string_to_cstr(str1), string_to_cstr(str2));
}

int string_equals(const String* str1, const String* str2) {
    return string_compare(str1, str2) == 0;
}

int string_equals_cstr(const String* str, const char* cstr) {
    if (!str || !cstr) { return 0; }
    return strcmp(string_to_cstr(str), cstr) == 0;
}

int string_find_char(const String* str, char c) {
    if (!str) { return -1; }
    
    const char* cstr = string_to_cstr(str);
    const char* found = strchr(cstr, c);
    return found ? found - cstr : -1;
}

int string_find_cstr(const String* str, const char* substring) {
    if (!str || !substring) { return -1; }
    
    const char* cstr = string_to_cstr(str);
    const char* found = strstr(cstr, substring);
    return found ? found - cstr : -1;
}

String* string_substr(const String* str, size_t start, size_t length) {
    if (!str || start >= string_len(str)) { return NULL; }
    
    String* result = string_create();
    if (!result) { return NULL; }
    
    size_t end = start + length;
    if (end > string_len(str)) {
        end = string_len(str);
    }
    
    for (size_t i = start; i < end; i++) {
        string_append_char(result, string_at(str, i));
    }
    
    return result;
}

void string_print(const String* str) {
    if (str) {
        printf("\"%s\"", string_to_cstr(str));
    }
}


