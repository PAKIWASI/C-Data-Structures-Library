#pragma once

#include "gen_vector.h"



typedef struct {
    genVec* buffer;  // Vector of chars - the actual string data
} String;



// Construction/Destruction
String*  string_create(void);
void     string_create_onstk(String* str, const char* cstr);
String*  string_from_cstr(const char* cstr);
String*  string_from_string(const String* other);
void     string_reserve(String* str, u32 capacity); 
void     string_destroy(String* str);
void     string_destroy_fromstk(String* str);


// get ptr to the cstr buffer
const char* string_to_cstr(const String* str);

// Modification
void string_append_cstr(String* str, const char* cstr);
void string_append_string(String* str, const String* other);
void string_append_char(String* str, char c);
void string_insert_char(String* str, u32 i, char c);
void string_insert_cstr(String* str, u32 i, const char* cstr);
void string_insert_string(String* str, u32 i, String* other);
char string_pop_char(String* str);
void string_remove_char(String* str, u32 i);
void string_remove_range(String* str, u32 l, u32 r);
void string_clear(String* str);

// Access
char string_at(const String* str, u32 i);
void string_set_char(String* str, u32 i, char c);

// Comparison
int string_compare(const String* str1, const String* str2);
int string_equals(const String* str1, const String* str2);
int string_equals_cstr(const String* str, const char* cstr);

// Search
u32 string_find_char(const String* str, char c);
u32 string_find_cstr(const String* str, const char* substr);

// Substring
String* string_substr(const String* str, u32 start, u32 length);

// I/O
void string_print(const String* str);

// Basic properties
static inline u32 string_len(const String* str) 
{
    if (!str || !str->buffer) { return 0; }

    u32 size = genVec_size(str->buffer);
    // Subtract 1 for null terminator if present
    return (size > 0) ? size - 1 : 0;
}
static inline u8 string_empty(const String* str) 
{
    return string_len(str) == 0;
}


