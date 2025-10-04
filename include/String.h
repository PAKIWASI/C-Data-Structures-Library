#pragma once

#include "gen_vector.h"
#include <stddef.h>


typedef struct {
    genVec* buffer;  // Vector of chars - the actual string data
} String;

// Construction/Destruction
String* string_create(void);
String* string_from_cstr(const char* cstr);
String* string_from_string(const String* other);
void string_reserve(String* str, size_t capacity); 
void string_destroy(String* str);

// Basic properties
size_t string_len(const String* str);
int string_empty(const String* str);
const char* string_to_cstr(const String* str);

// Modification
void string_append_cstr(String* str, const char* cstr);
void string_append_string(String* str, const String* other);
void string_append_char(String* str, char c);
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
int string_find_cstr(const String* str, const char* substring);

// Substring
String* string_substr(const String* str, size_t start, size_t length);

// I/O
void string_print(const String* str);


