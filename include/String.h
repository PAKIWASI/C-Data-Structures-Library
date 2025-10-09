#pragma once

#include "gen_vector.h"
#include <stddef.h>


typedef struct {
    genVec* buffer;  // Vector of chars - the actual string data
} String;

// Construction/Destruction
String* string_create(void);
void string_create_onstack(String* str, const char* cstr);
String* string_from_cstr(const char* cstr);
String* string_from_string(const String* other);
void string_reserve(String* str, size_t capacity); 
void string_destroy(String* str);
void string_destroy_fromstk(String* str);

// Basic properties
static inline size_t string_len(const String* str) {
    if (!str || !str->buffer) { return 0; }

    size_t size = genVec_size(str->buffer);
    // Subtract 1 for null terminator if present
    return (size > 0) ? size - 1 : 0;
}
static inline int string_empty(const String* str) {
    return string_len(str) == 0;
}
const char* string_to_cstr(const String* str);

// Modification
void string_append_cstr(String* str, const char* cstr);
void string_append_string(String* str, const String* other);
void string_append_char(String* str, char c);
void string_insert_char(String* str, size_t i, char c);
void string_insert_cstr(String* str, size_t i, const char* cstr);
void string_insert_string(String* str, size_t i, String* other);
void string_remove_char(String* str, size_t i);
void string_clear(String* str);

// Access
char string_at(const String* str, size_t i);
void string_set_char(String* str, size_t i, char c);

// Comparison
int string_compare(const String* str1, const String* str2);
int string_equals(const String* str1, const String* str2);
int string_equals_cstr(const String* str, const char* cstr);

// Search
int string_find_char(const String* str, char c);
int string_find_cstr(const String* str, const char* substr);

// Substring
String* string_substr(const String* str, size_t start, size_t length);

// I/O
void string_print(const String* str);


