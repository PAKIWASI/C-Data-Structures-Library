#pragma once

#include "gen_vector.h"


typedef struct {
    genVec buffer; // Vector of chars - the actual string data
} String;


// Construction/Destruction

String* string_create(void);
void string_create_stk(String* str, const char* cstr, u32 len);
String* string_from_cstr(const char* cstr, u32 len);
// get copy of a string (heap allocated)
String* string_from_string(const String* other);
void    string_reserve(String* str, u32 capacity);
void    string_destroy(String* str);
void    string_destroy_stk(String* str);

// move string contents (nulls source)
// Note: src must be heap allocated
void string_move(String* dest, String** src);

// make deep copy
void string_copy(String* dest, const String* src);


// get cstr as COPY ('\0' present)
const char* string_to_cstr(const String* str);
// get ptr to the cstr buffer
// Note: NO NULL TERMINATOR
char* string_data_ptr(const String* str);

// TODO: 
void string_to_cstr_buf(const String* str, char* buf, u32 buf_size);
void string_to_cstr_buf_move(const String* str, char* buf, u32 buf_size);

// Modification
void string_append_cstr(String* str, const char* cstr, u32 len);
void string_append_string(String* str, const String* other);
// Concatenate and destroy source
void string_append_string_move(String* str, String** other);

void string_append_char(String* str, char c);
void string_insert_char(String* str, u32 i, char c);
void string_insert_cstr(String* str, u32 i, const char* cstr, u32 len);
void string_insert_string(String* str, u32 i, const String* other);
char string_pop_char(String* str);
void string_remove_char(String* str, u32 i);
void string_remove_range(String* str, u32 l, u32 r);
void string_clear(String* str);


// Access
char string_char_at(const String* str, u32 i);
void string_set_char(String* str, u32 i, char c);

// Comparison
// 0 -> equal, 1 -> not equal
int string_compare(const String* str1, const String* str2);
b8  string_equals(const String* str1, const String* str2);
b8 string_equals_cstr(const String* str, const char* cstr, u32 len);

// Search
u32 string_find_char(const String* str, char c);
u32 string_find_cstr(const String* str, const char* substr, u32 len);

// Substring
String* string_substr(const String* str, u32 start, u32 length);

// I/O
void string_print(const String* str);

// Basic properties
static inline u32 string_len(const String* str)
{
    CHECK_FATAL(!str, "str is null");

    return str->buffer.size;
}

static inline u32 string_capacity(const String* str) 
{ 
    return str->buffer.capacity; 
}

static inline b8 string_empty(const String* str)
{
    return string_len(str) == 0;
}


// TODO: 

/*
// Split string by delimiter
String** string_split(const String* str, char delim, u32* out_count);

// Join array of strings
String* string_join(String** strings, u32 count, const char* sep);

// Trim whitespace
void string_trim(String* str);
void string_trim_left(String* str);
void string_trim_right(String* str);

// Case conversion
void string_to_upper(String* str);
void string_to_lower(String* str);

// Replace substring
u32 string_replace(String* str, const char* old, const char* new);

// Format string (like sprintf)
void string_format(String* str, const char* fmt, ...);
String* string_format_new(const char* fmt, ...);

// Reverse string
void string_reverse(String* str);

// Check if starts/ends with
b8 string_starts_with(const String* str, const char* prefix);
b8 string_ends_with(const String* str, const char* suffix);

// Contains substring
b8 string_contains(const String* str, const char* substr);

// Count occurrences
u32 string_count_char(const String* str, char c);

// Repeat string
String* string_repeat(const String* str, u32 times);
*/
