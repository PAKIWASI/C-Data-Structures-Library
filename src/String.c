#include "String.h"
#include "common.h"
#include "gen_vector.h"

#include <string.h>



// Private helper to ensure null termination
static inline void ensure_null_terminated(String* str) 
{
    if (!str || !str->buffer) { return; }
    
    u32 size = str->buffer->size;
    if (size == 0 || (str->buffer->data)[size - 1] != '\0') 
    {
        const char null_term = '\0';
        genVec_push(str->buffer, (u8*)&null_term);
    }
}

String* string_create(void) 
{
    String* str = malloc(sizeof(String));
    CHECK_FATAL(!str, "malloc failed");
    
    // we get a pointer to buffer
    str->buffer = genVec_init(0, sizeof(char), NULL);
    if (!str->buffer) {
        free(str);
        FATAL("genVec_init failed");
    }
    
    ensure_null_terminated(str);
    return str;
}


void string_create_onstk(String* str, const char* cstr) 
{
    // the difference is that we dont use string_create(), so str is not heap initilised
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    str->buffer = genVec_init(0, sizeof(char), NULL);
    CHECK_FATAL(!str->buffer, "buffer init failed");
    
    string_append_cstr(str, cstr); 
}

String* string_from_cstr(const char* cstr) 
{
    CHECK_FATAL(!cstr, "cstr is null");
    
    String* str = string_create();
    CHECK_FATAL(!str, "string_create failed");
    
    string_append_cstr(str, cstr);
    return str;
}

String* string_from_string(const String* other) 
{
    CHECK_FATAL(!other, "other is null");

    return string_from_cstr(string_to_cstr(other));
}

void string_reserve(String* str, u32 capacity) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!str->buffer, "buffer is null");
    
    // Add 1 for null terminator
    capacity++;
    
    // Use genVec_reserve to pre-allocate
    genVec_reserve(str->buffer, capacity);
    
    // Ensure null termination is preserved
    ensure_null_terminated(str);
}

void string_destroy(String* str) 
{
    string_destroy_fromstk(str);

    free(str);
}

// cant free the stack allocated string, but buffer is heap. So seperate delete 
void string_destroy_fromstk(String* str) 
{
    CHECK_FATAL(!str, "str is null");

    genVec_destroy(str->buffer);
}

// TODO: should return a copy ?
const char* string_to_cstr(const String* str) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!str->buffer, "str buffer is null");

    if (str->buffer->size == 0) {
        return "";
    }
    
    return (const char*)str->buffer->data;
}

void string_append_cstr(String* str, const char* cstr) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!str->buffer, "str buffer is null");
    CHECK_FATAL(!cstr, "cstr is null");
    
    u32 cstr_len = strlen(cstr);
    if (cstr_len == 0) { return; }// Nothing to append

    // Calculate where to start writing (before null terminator (if it exists))
    u32 write_pos = str->buffer->size;
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

void string_append_string(String* str, const String* other) 
{
    CHECK_FATAL(!str, "str is empty");
    CHECK_FATAL(!other, "other is empty");

    string_append_cstr(str, string_to_cstr(other));
}

void string_append_char(String* str, char c) 
{
    CHECK_FATAL(!str, "str is null");
    
    // Remove null terminator temporarily
    if (str->buffer->size > 0) {
        genVec_pop(str->buffer, NULL);
    }
    
    genVec_push(str->buffer, (u8*)&c);
    ensure_null_terminated(str);
}

char string_pop_char(String* str)
{
    CHECK_FATAL(!str, "str is null");

    // Remove null terminator temporarily
    if (str->buffer->size > 0) {
        genVec_pop(str->buffer, NULL);
    }
    
    char c;
    genVec_pop(str->buffer, (u8*)&c);
    ensure_null_terminated(str);

    return c;
}

void string_insert_char(String* str, u32 i, char c)
{
    CHECK_FATAL(!str, "str is null");

    if (i >= string_len(str)) {
        string_append_char(str, c); // this will ensure null term at end
        return;
    } 
    
    genVec_insert(str->buffer, i, (u8*)&c); 
    ensure_null_terminated(str);
}


void string_insert_cstr(String* str, u32 i, const char* cstr)
{
    CHECK_FATAL(!str, "str is null");

    if (i >= string_len(str)) {
        string_append_cstr(str, cstr); // null term garenteed
        return;
    }
    
    genVec_insert_multi(str->buffer, i, (u8*)cstr, strlen(cstr));
    ensure_null_terminated(str);
}

void string_insert_string(String* str, u32 i, String* other)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!other, "other is null");

    if (i >= string_len(str)) {
        string_append_string(str, other);
        return;
    }
    
    // TODO: check this
    string_insert_cstr(str, i, string_to_cstr(other));
}


void string_remove_char(String* str, u32 i) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!str->buffer, "str buffer is null");

    CHECK_FATAL(i >= string_len(str), "index out of bounds");
    
    // Remove the character at index i
    genVec_remove(str->buffer, i);
    
    // The null terminator should still be there, but im scared
    ensure_null_terminated(str);
}

void string_remove_range(String* str, u32 l, u32 r)
{
    CHECK_FATAL(!str, "str is null");

    CHECK_FATAL(l >= string_len(str), "index out of bounds");

    genVec_remove_range(str->buffer, l, r);

    // upper func sets r to len - 1 so null terminator will be removed
    ensure_null_terminated(str);
}

void string_clear(String* str) 
{
    CHECK_FATAL(!str, "str is null");
    
    genVec_clear(str->buffer);
    ensure_null_terminated(str);
}

char string_at(const String* str, u32 i) 
{
    CHECK_FATAL(!str, "str is null");

    CHECK_FATAL(i >= string_len(str), "index out of bounds");

    char c;
    genVec_get(str->buffer, i, (u8*)&c);

    return c;
}

void string_set_char(String* str, u32 i, char c) 
{
    CHECK_FATAL(!str, "str is null");

    CHECK_FATAL(i >= string_len(str), "index out of bounds");

    genVec_replace(str->buffer, i, (u8*)&c);
}

int string_compare(const String* str1, const String* str2) 
{
    CHECK_FATAL(!str1, "str1 is null");
    CHECK_FATAL(!str2, "str2 is null");

    return strcmp(string_to_cstr(str1), string_to_cstr(str2));
}

int string_equals(const String* str1, const String* str2) 
{
    return string_compare(str1, str2) == 0;
}

int string_equals_cstr(const String* str, const char* cstr) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    return strcmp(string_to_cstr(str), cstr) == 0;
}

u32 string_find_char(const String* str, char c) 
{
    CHECK_FATAL(!str, "str is null");
    
    const char* cstr = string_to_cstr(str);
    const char* found = strchr(cstr, c);

    // found points to index we want and cstr points to 0th pos
    return found ? found - cstr : -1;
}

u32 string_find_cstr(const String* str, const char* substr) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!substr, "substr is null");
    
    const char* cstr = string_to_cstr(str);
    const char* found = strstr(cstr, substr);
    return found ? found - cstr : -1;
}


String* string_substr(const String* str, u32 start, u32 length) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(start >= string_len(str), "index out of bounds");
    
    String* result = string_create();
    
    u32 end = start + length;
    u32 str_len = string_len(str);
    if (end > str_len) {
        end = str_len;
    }
    
    u32 actual_len = end - start;
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

void string_print(const String* str) 
{
    CHECK_FATAL(!str, "str is null");

    printf("\"%s\"", string_to_cstr(str));
}


