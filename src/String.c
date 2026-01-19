#include "String.h"
#include "common.h"

#include <string.h>


String* string_create(void)
{
    String* str = malloc(sizeof(String));
    CHECK_FATAL(!str, "malloc failed");

    genVec_init_stk(0, sizeof(char), NULL, NULL, NULL, &str->buffer);

    return str;
}

void string_create_stk(String* str, const char* cstr)
{
    // the difference is that we dont use string_create(), so str is not heap initilised
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    genVec_init_stk(0, sizeof(char), NULL, NULL, NULL, &str->buffer);

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

    String* str = string_create();

    if (other->buffer.size > 0) { // Direct copy of buffer
        genVec_insert_multi(&str->buffer, 0, other->buffer.data, other->buffer.size);
    }

    return str;
}

void string_reserve(String* str, u32 capacity)
{
    CHECK_FATAL(!str, "str is null");
    genVec_reserve(&str->buffer, capacity);
}

void string_destroy(String* str)
{
    string_destroy_stk(str);
    free(str);
}

// cant free the stack allocated string, but buffer is heap. So seperate delete
void string_destroy_stk(String* str)
{
    CHECK_FATAL(!str, "str is null");
    genVec_destroy_stk(&str->buffer);
}

void string_move(String* dest, String** src)
{
    CHECK_FATAL(!src, "src is null");
    CHECK_FATAL(!*src, "src is null");
    CHECK_FATAL(!dest, "dest is null");

    if (dest == *src) {
        *src = NULL;
        return;
    }

    // no op if dest's data ptr is null (like stack inited)
    string_destroy_stk(dest);

    // copy fields (including data ptr)
    memcpy(dest, *src, sizeof(String));
    
    (*src)->buffer.data = NULL;
    free(*src);
    *src = NULL;
}


void string_copy(String* dest, String* src)
{
    CHECK_FATAL(!src, "src is null");
    CHECK_FATAL(!dest, "dest is null");

    if (src == dest) { return; }

    // no op if data ptr is null
    string_destroy_stk(dest);

    // copy all fields (data ptr too)
    memcpy(dest, src, sizeof(String));

    // malloc new data ptr
    dest->buffer.data = malloc(src->buffer.size);

    // copy all data (arr of chars)
    memcpy(dest->buffer.data, src->buffer.data, src->buffer.size);
}


const char* string_to_cstr(const String* str)
{
    CHECK_FATAL(!str, "str is null");

    if (str->buffer.size == 0) {
        char* empty = malloc(1);
        CHECK_FATAL(!empty, "malloc failed");
        empty[0] = '\0';
        return empty;
    }

    char* out = malloc(str->buffer.size + 1); // + 1 for null term
    CHECK_FATAL(!out, "out str malloc failed");

    memcpy(out, genVec_front(&str->buffer), str->buffer.size);

    out[str->buffer.size] = '\0'; // add null term

    return (const char*)out;
}


const char* string_to_cstr_ptr(const String* str)
{
    CHECK_FATAL(!str, "str is null");

    if (str->buffer.size == 0) { return NULL; }

    return (const char*)str->buffer.data;
}


void string_append_cstr(String* str, const char* cstr)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    u32 cstr_len = (u32)strlen(cstr);
    if (cstr_len == 0) { return; }

    genVec_insert_multi(&str->buffer, str->buffer.size, (const u8*)cstr, cstr_len);
}


void string_append_string(String* str, const String* other)
{
    CHECK_FATAL(!str, "str is empty");
    CHECK_FATAL(!other, "other is empty");

    if (other->buffer.size == 0) { return; }

    // direct insertion from other's buffer
    genVec_insert_multi(&str->buffer, str->buffer.size, other->buffer.data, other->buffer.size);
}

// append and consume source string
void string_append_string_move(String* str, String** other)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!other, "other ptr is null");
    CHECK_FATAL(!*other, "*other is null");

    if ((*other)->buffer.size > 0) {
        genVec_insert_multi(&str->buffer, str->buffer.size, 
                            (*other)->buffer.data, (*other)->buffer.size);
    }

    string_destroy(*other);
    *other = NULL;
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

    u32 len = (u32)strlen(cstr);
    if (len == 0) { return; }

    genVec_insert_multi(&str->buffer, i, castptr(cstr), len);
}

void string_insert_string(String* str, u32 i, const String* other)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!other, "other is null");
    CHECK_FATAL(i > str->buffer.size, "index out of bounds");

    if (other->buffer.size == 0) { return; }

    // direct insertion
    genVec_insert_multi(&str->buffer, i, other->buffer.data, other->buffer.size);
}

void string_remove_char(String* str, u32 i)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(i >= str->buffer.size, "index out of bounds");

    genVec_remove(&str->buffer, i, NULL);
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

    return ((char*)str->buffer.data)[i];
}

void string_set_char(String* str, u32 i, char c)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(i >= str->buffer.size, "index out of bounds");

    ((char*)str->buffer.data)[i] = c;
}

int string_compare(const String* str1, const String* str2)
{
    CHECK_FATAL(!str1, "str1 is null");
    CHECK_FATAL(!str2, "str2 is null");

    u32 min_len = str1->buffer.size < str2->buffer.size ? str1->buffer.size : str2->buffer.size;

    // Compare byte by byte
    int cmp = memcmp(str1->buffer.data, str2->buffer.data, min_len);

    if (cmp != 0) { return cmp; }

    // If equal so far, shorter string is "less"
    if (str1->buffer.size < str2->buffer.size) { return -1; }
    if (str1->buffer.size > str2->buffer.size) { return 1; }

    return 0;
}

b8 string_equals(const String* str1, const String* str2)
{
    return string_compare(str1, str2) == 0;
}

b8 string_equals_cstr(const String* str, const char* cstr)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    u32 cstr_len = (u32)strlen(cstr);

    if (str->buffer.size != cstr_len) { return 0; }

    return memcmp(str->buffer.data, cstr, cstr_len) == 0;
}

u32 string_find_char(const String* str, char c)
{
    CHECK_FATAL(!str, "str is null");

    for (u32 i = 0; i < str->buffer.size; i++) {
        if (((char*)str->buffer.data)[i] == c) { return i; }
    }

    return (u32)-1; // Not found
}

u32 string_find_cstr(const String* str, const char* substr)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!substr, "substr is null");

    u32 substr_len = (u32)strlen(substr);
    if (substr_len == 0 || substr_len > str->buffer.size) { return (u32)-1; }

    for (u32 i = 0; i <= str->buffer.size - substr_len; i++) {
        if (memcmp(str->buffer.data + i, substr, substr_len) == 0) { return i; }
    }

    return (u32)-1;
}

String* string_substr(const String* str, u32 start, u32 length)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(start >= str->buffer.size, "index out of bounds");

    String* result = string_create();

    u32 end     = start + length;
    u32 str_len = string_len(str);
    if (end > str_len) { end = str_len; }

    u32 actual_len = end - start;

    if (actual_len > 0) { // Insert substring all at once
        const char* csrc = string_to_cstr_ptr(str) + start;
        genVec_insert_multi(&result->buffer, 0, (const u8*)csrc, actual_len);
    }

    return result;
}

void string_print(const String* str)
{
    CHECK_FATAL(!str, "str is null");

    printf("\"");
    for (u32 i = 0; i < str->buffer.size; i++) { putchar(((char*)str->buffer.data)[i]); }
    printf("\"");
}



