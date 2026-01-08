#include "String.h"
#include "common.h"
#include "gen_vector.h"

#include <string.h>


String* string_create(void)
{
    String* str = malloc(sizeof(String));
    CHECK_FATAL(!str, "malloc failed");
    
    //str->buffer = genVec_init(0, sizeof(char), NULL);
    genVec_init_stk(0, sizeof(char), NULL, &str->buffer);
    
    return str;
}

void string_create_onstk(String* str, const char* cstr) 
{
    // the difference is that we dont use string_create(), so str is not heap initilised
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    //str->buffer = genVec_init(0, sizeof(char), NULL);
    genVec_init_stk(0, sizeof(char), NULL, &str->buffer);
    
    string_append_cstr(str, cstr); 
}

String* string_from_cstr(const char* cstr) 
{
    CHECK_FATAL(!cstr, "cstr is null");
    
    String* str = string_create();
    
    string_append_cstr(str, cstr);

    return str;
}

String* string_from_string(const String* other) 
{
    CHECK_FATAL(!other, "other str is null");

    const char* cstr = string_to_cstr(other); // copy
    String* out = string_from_cstr(cstr);
    free((void*)cstr);
    return out;
}

void string_reserve(String* str, u32 capacity) 
{
    CHECK_FATAL(!str, "str is null");
    
    // Use genVec_reserve to pre-allocate
    genVec_reserve(&str->buffer, capacity);
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

    genVec_destroy_stk(&str->buffer);
}

// TODO: new func (need to test)
const char* string_to_cstr(const String* str)
{
    CHECK_FATAL(!str, "str is null");

    if (str->buffer.size == 0) {
        return "";
    }

    char* out = malloc(str->buffer.size + 1); // + 1 for null term
    CHECK_FATAL(!out, "out str malloc failed");
    
    memcpy(out, genVec_get_data(&str->buffer),str->buffer.size);

    out[str->buffer.size] = '\0'; // add null term 

    return (const char*) out;
}

// WARN: no null term!
const char* string_to_cstr_ptr(const String* str) 
{
    CHECK_FATAL(!str, "str is null");

    if (str->buffer.size == 0) {
        return NULL;
    }
    
    return (const char*)str->buffer.data;
}

void string_append_cstr(String* str, const char* cstr) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");
    
    u32 cstr_len = strlen(cstr); // TODO: does not count null term
    if (cstr_len == 0) { return; }// nothing to append

    genVec_insert_multi(&str->buffer, str->buffer.size, (const u8*)cstr, cstr_len);
}

void string_append_string(String* str, const String* other) 
{
    CHECK_FATAL(!str, "str is empty");
    CHECK_FATAL(!other, "other is empty");

    const char* cstr = string_to_cstr(other);   // using string_to_cstr which returns a COPY
    string_append_cstr(str, cstr);
    free((void*)cstr); // this was copied
}

void string_append_char(String* str, char c) 
{
    CHECK_FATAL(!str, "str is null");
    
    genVec_push(&str->buffer, cast(c));
}

char string_pop_char(String* str)
{
    CHECK_FATAL(!str, "str is null");

    char c;
    genVec_pop(&str->buffer, cast(c));

    return c;
}

void string_insert_char(String* str, u32 i, char c)
{
    CHECK_FATAL(!str, "str is null");

    CHECK_FATAL(i > str->buffer.size, "index out of bounds");
    
    genVec_insert(&str->buffer, i, cast(c)); 
}

void string_insert_cstr(String* str, u32 i, const char* cstr)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    CHECK_FATAL(i > str->buffer.size, "index out of bounds");

    u32 len = strlen(cstr);
    if (len ==  0) { return; }
    
    genVec_insert_multi(&str->buffer, i, castptr(cstr), len);
}

void string_insert_string(String* str, u32 i, String* other)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!other, "other is null");

    CHECK_FATAL(i > str->buffer.size, "index out of bounds");
    
    string_insert_cstr(str, i, string_to_cstr_ptr(other));
}

void string_remove_char(String* str, u32 i) 
{
    CHECK_FATAL(!str, "str is null");

    CHECK_FATAL(i >= str->buffer.size, "index out of bounds");
    
    // Remove the character at index i
    genVec_remove(&str->buffer, i);
}

void string_remove_range(String* str, u32 l, u32 r)
{
    CHECK_FATAL(!str, "str is null");

    CHECK_FATAL(l >= str->buffer.size, "index out of bounds");

    CHECK_FATAL(l > r, "invalid range");

    genVec_remove_range(&str->buffer, l, r);
}

void string_clear(String* str) 
{
    CHECK_FATAL(!str, "str is null");
    
    genVec_clear(&str->buffer);
}

char string_char_at(const String* str, u32 i) 
{
    CHECK_FATAL(!str, "str is null");

    CHECK_FATAL(i >= str->buffer.size, "index out of bounds");

    char c;
    genVec_get(&str->buffer, i, cast(c));

    return c;
}

void string_set_char(String* str, u32 i, char c) 
{
    CHECK_FATAL(!str, "str is null");

    CHECK_FATAL(i >= str->buffer.size, "index out of bounds");

    genVec_replace(&str->buffer, i, cast(c));
}

int string_compare(String* str1, String* str2) 
{
    CHECK_FATAL(!str1, "str1 is null");
    CHECK_FATAL(!str2, "str2 is null");

    string_append_char(str1, '\0');
    string_append_char(str2, '\0');

    int ret =  strcmp(string_to_cstr_ptr(str1), string_to_cstr_ptr(str2));

    string_pop_char(str1);
    string_pop_char(str2);

    return ret;
}

b8 string_equals(String* str1, String* str2) 
{
    return string_compare(str1, str2) == 0;
}

b8 string_equals_cstr(String* str, const char* cstr) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    string_append_char(str, '\0');

    int ret = strcmp(string_to_cstr_ptr(str), cstr) == 0;

    string_pop_char(str);

    return ret;
}

u32 string_find_char(String* str, char c) 
{
    CHECK_FATAL(!str, "str is null");
    
    string_append_char(str, '\0'); 

    const char* cstr = string_to_cstr_ptr(str);
    const char* found = strchr(cstr, c);

    string_pop_char(str);

    // found points to index we want and cstr points to 0th pos
    return found ? found - cstr : -1;
}

u32 string_find_cstr(String* str, const char* substr) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!substr, "substr is null");
    
    string_append_char(str, '\0');

    const char* cstr = string_to_cstr_ptr(str);
    const char* found = strstr(cstr, substr);
    
    string_pop_char(str);

    return found ? found - cstr : -1;
}

String* string_substr(const String* str, u32 start, u32 length) 
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(start >= str->buffer.size, "index out of bounds");
    
    String* result = string_create();
    
    u32 end = start + length;
    u32 str_len = string_len(str);
    if (end > str_len) {
        end = str_len;
    }
    
    u32 actual_len = end - start;
        
    if (actual_len > 0) {
        // Insert substring all at once
        const char* csrc = string_to_cstr_ptr(str) + start;
        genVec_insert_multi(&result->buffer, 0, (const u8*)csrc, actual_len);
    }
    
    return result;
}

void string_print(String* str) 
{
    CHECK_FATAL(!str, "str is null");

    string_append_char(str, '\0');
    printf("\"%s\"", string_to_cstr_ptr(str));
    string_pop_char(str);
}


