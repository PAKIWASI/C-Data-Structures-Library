#include "String.h"
#include "gen_vector.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



// Private helper to ensure null termination
static inline void ensure_null_terminated(String* str) {
    if (!str || !str->buffer) { return; }
    
    size_t size = str->buffer->size;
    if (size == 0 || ((char*)str->buffer->data)[size - 1] != '\0') 
    {
        const char null_term = '\0';
        genVec_push(str->buffer, (u8*)&null_term);
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


void string_create_onstack(String* str, const char* cstr) 
{
    // the difference is that we dont use string_create(), so str is not heap initilised
    if (!str) {
        printf("str create stk: str is null\n");
        return;
    }

    str->buffer = genVec_init(0, sizeof(char), NULL);
    if (!str->buffer) {
        printf("str create stk: buffer genvec failed\n");
        return;
    }
    
    string_append_cstr(str, cstr); 
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

// cant free the stack allocated string, but buffer is heap. So seperate delete 
void string_destroy_fromstk(String* str) {
    if (str->buffer) {
        genVec_destroy(str->buffer);
    }
}

const char* string_to_cstr(const String* str) {
    if (!str || !str->buffer || str->buffer->size == 0) { 
        return ""; 
    }
    
    return (const char*)str->buffer->data;
}

void string_append_cstr(String* str, const char* cstr) 
{
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
        genVec_get(str->buffer, write_pos - 1, (u8*)&last_char);
        if (last_char == '\0') {
            write_pos--; // Overwrite the null terminator
            str->buffer->size--; // Temporarily reduce size
        }
    }
    
    genVec_insert_multi(str->buffer, write_pos, (const u8*)cstr, cstr_len);
    
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
    
    genVec_push(str->buffer, (u8*)&c);
    ensure_null_terminated(str);
}

void string_insert_char(String* str, size_t i, char c)
{
    if (!str) {
        printf("str insert char: str is null\n");
        return;
    } 
    if (i >= string_len(str)) {
        string_append_char(str, c); // this will ensure null term at end
        return;
    } 
    
    genVec_insert(str->buffer, i, (u8*)&c); 
    ensure_null_terminated(str);
}


void string_insert_cstr(String* str, size_t i, const char* cstr)
{
    if (!str) {
        printf("str insert cstr: str is null\n");
        return;
    }
    if (i >= string_len(str)) {
        string_append_cstr(str, cstr); // null term garenteed
        return;
    }
    
    genVec_insert_multi(str->buffer, i, (u8*)cstr, strlen(cstr));
    ensure_null_terminated(str);
}

void string_insert_string(String* str, size_t i, String* other)
{
    if (!str || !other) {
        printf("str insert str: parameters null\n");
        return;
    }
    if (i >= string_len(str)) {
        string_append_string(str, other);
        return;
    }
    
    // is this right???
    genVec_insert_multi(str->buffer, i, other->buffer->data, strlen(string_to_cstr(other)));
    ensure_null_terminated(str);
}


void string_remove_char(String* str, size_t i) {
    if (!str || !str->buffer) { 
        printf("str remove char: str or buffer null\n");
        return; 
    }
    if (i >= string_len(str)) {
        printf("str remove char: index out of bounds\n");
        return;
    }
    
    // Remove the character at index i
    genVec_remove(str->buffer, i);
    
    // The null terminator should still be there, but im scared
    ensure_null_terminated(str);
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
    genVec_get(str->buffer, i, (u8*)&c);
    return c;
}

void string_set_char(String* str, size_t i, char c) {
    if (!str || i >= string_len(str)) { 
        printf("str set char: str null or i out of bounds\n");
        return; 
    }
    genVec_replace(str->buffer, i, (u8*)&c);
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

int string_find_cstr(const String* str, const char* substr) {
    if (!str || !substr) { return -1; }
    
    const char* cstr = string_to_cstr(str);
    const char* found = strstr(cstr, substr);
    return found ? found - cstr : -1;
}

String* string_substr(const String* str, size_t start, size_t length) 
{
    if (!str || start >= string_len(str)) { return NULL; }
    
    String* result = string_create();
    if (!result) { return NULL; }
    
    size_t end = start + length;
    size_t str_len = string_len(str);
    if (end > str_len) {
        end = str_len;
    }
    
    size_t actual_len = end - start;
    if (actual_len > 0) {
        // Remove null terminator from result
        if (result->buffer->size > 0) {
            result->buffer->size--;
        }
        
        // Insert substring all at once
        const char* src = string_to_cstr(str) + start;
        genVec_insert_multi(result->buffer, 0, (const u8*)src, actual_len);
        
        ensure_null_terminated(result);
    }
    return result;
}

void string_print(const String* str) {
    if (str) {
        printf("\"%s\"", string_to_cstr(str));
    }
}


