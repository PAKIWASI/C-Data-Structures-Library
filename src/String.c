#include "String.h"

#include <string.h>




// private func
u32 cstr_len(const char* cstr);



String* string_create(void)
{
    return genVec_init(0, sizeof(char), NULL, NULL, NULL);
}


void string_create_stk(String* str, const char* cstr)
{
    // the difference is that we dont use string_create(), so str is not initilised
    CHECK_FATAL(!str, "str is null");

    u32 len = 0;
    if (cstr) {
        len = cstr_len(cstr);
    }

    genVec_init_stk(len, sizeof(char), NULL, NULL, NULL, str);
    
    if (len != 0) {
        genVec_insert_multi(str, 0, (const u8*)cstr, len);
    }
}


String* string_from_cstr(const char* cstr)
{
    String* str = malloc(sizeof(String));
    CHECK_FATAL(!str, "str malloc failed");

    string_create_stk(str, cstr);
    return str;
}


String* string_from_string(const String* other)
{
    CHECK_FATAL(!other, "other str is null");

    String* str = malloc(sizeof(String));
    CHECK_FATAL(!str, "str malloc failed");

    genVec_init_stk(other->size, sizeof(char), NULL, NULL, NULL, str);

    if (other->size != 0) {
        genVec_insert_multi(str, 0, other->data, other->size);
    }

    return str;
}


void string_reserve(String* str, u32 capacity)
{
    genVec_reserve(str, capacity);
}


void string_reserve_char(String* str, u32 capacity, char c)
{
    genVec_reserve_val(str, capacity, cast(c));
}


void string_destroy(String* str)
{
    string_destroy_stk(str);
    free(str);
}

// cant free the stack allocated string, but buffer is. So seperate delete
void string_destroy_stk(String* str)
{
    genVec_destroy_stk(str);
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

    (*src)->data = NULL;
    free(*src);
    *src = NULL;
}


void string_copy(String* dest, const String* src)
{
    CHECK_FATAL(!src, "src is null");
    CHECK_FATAL(!dest, "dest is null");

    if (src == dest) {
        return;
    }

    // no op if data ptr is null
    string_destroy_stk(dest);

    // copy all fields (data ptr too)
    memcpy(dest, src, sizeof(String));

    // malloc new data ptr
    dest->data = malloc(src->capacity);

    // copy all data (arr of chars)
    memcpy(dest->data, src->data, src->size);
}


const char* string_to_cstr(const String* str)
{
    CHECK_FATAL(!str, "str is null");

    if (str->size == 0) {
        char* empty = malloc(1);
        CHECK_FATAL(!empty, "malloc failed");
        empty[0] = '\0';
        return empty;
    }

    char* out = malloc(str->size + 1); // + 1 for null term
    CHECK_FATAL(!out, "out str malloc failed");

    memcpy(out, str->data, str->size);

    out[str->size] = '\0'; // add null term

    return (const char*)out;
}


char* string_data_ptr(const String* str)
{
    CHECK_FATAL(!str, "str is null");

    if (str->size == 0) {
        return NULL;
    }

    return (char*)str->data;
}


void string_append_cstr(String* str, const char* cstr)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");

    u32 len = cstr_len(cstr);
    if (len == 0) {
        return;
    }

    genVec_insert_multi(str, str->size, (const u8*)cstr, len);
}


void string_append_string(String* str, const String* other)
{
    CHECK_FATAL(!str, "str is empty");
    CHECK_FATAL(!other, "other is empty");

    if (other->size == 0) {
        return;
    }

    // direct insertion from other's buffer
    genVec_insert_multi(str, str->size, other->data, other->size);
}

// append and consume source string
void string_append_string_move(String* str, String** other)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!other, "other ptr is null");
    CHECK_FATAL(!*other, "*other is null");

    if ((*other)->size > 0) {
        genVec_insert_multi(str, str->size, (*other)->data, (*other)->size);
    }

    string_destroy(*other);
    *other = NULL;
}


void string_append_char(String* str, char c)
{
    CHECK_FATAL(!str, "str is null");
    genVec_push(str, cast(c));
}


char string_pop_char(String* str)
{
    CHECK_FATAL(!str, "str is null");

    char c;
    genVec_pop(str, cast(c));

    return c;
}


void string_insert_char(String* str, u32 i, char c)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(i > str->size, "index out of bounds");

    genVec_insert(str, i, cast(c));
}


void string_insert_cstr(String* str, u32 i, const char* cstr)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!cstr, "cstr is null");
    CHECK_FATAL(i > str->size, "index out of bounds");

    u32 len = cstr_len(cstr);
    if (len == 0) {
        return;
    }

    genVec_insert_multi(str, i, castptr(cstr), len);
}


void string_insert_string(String* str, u32 i, const String* other)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!other, "other is null");
    CHECK_FATAL(i > str->size, "index out of bounds");

    if (other->size == 0) {
        return;
    }

    // direct insertion
    genVec_insert_multi(str, i, other->data, other->size);
}


void string_remove_char(String* str, u32 i)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(i >= str->size, "index out of bounds");

    genVec_remove(str, i, NULL);
}


void string_remove_range(String* str, u32 l, u32 r)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(l >= str->size, "index out of bounds");
    CHECK_FATAL(l > r, "invalid range");

    genVec_remove_range(str, l, r);
}


void string_clear(String* str)
{
    CHECK_FATAL(!str, "str is null");
    genVec_clear(str);
}


char string_char_at(const String* str, u32 i)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(i >= str->size, "index out of bounds");

    return ((char*)str->data)[i];
}


void string_set_char(String* str, u32 i, char c)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(i >= str->size, "index out of bounds");

    ((char*)str->data)[i] = c;
}


int string_compare(const String* str1, const String* str2)
{
    CHECK_FATAL(!str1, "str1 is null");
    CHECK_FATAL(!str2, "str2 is null");

    u32 min_len = str1->size < str2->size ? str1->size : str2->size;

    // Compare byte by byte
    int cmp = memcmp(str1->data, str2->data, min_len);

    if (cmp != 0) {
        return cmp;
    }

    // If equal so far, shorter string is "less"
    if (str1->size < str2->size) {
        return -1;
    }
    if (str1->size > str2->size) {
        return 1;
    }

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

    u32 len = cstr_len(cstr);

    // Different lengths = not equal
    if (str->size != len) {
        return false;
    }
    // Both empty
    if (len == 0) {
        return true;
    }

    return memcmp(str->data, cstr, len) == 0;
}


u32 string_find_char(const String* str, char c)
{
    CHECK_FATAL(!str, "str is null");

    for (u32 i = 0; i < str->size; i++) {
        if (((char*)str->data)[i] == c) {
            return i;
        }
    }

    return (u32)-1; // Not found
}


u32 string_find_cstr(const String* str, const char* substr)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(!substr, "substr is null");

    u32 len = cstr_len(substr);

    // Empty substring is found at index 0
    if (len == 0) {
        return 0;
    }

    if (len > str->size) {
        return (u32)-1;
    }

    for (u32 i = 0; i <= str->size - len; i++) {
        if (memcmp(str->data + i, substr, len) == 0) {
            return i;
        }
    }

    return (u32)-1;
}


String* string_substr(const String* str, u32 start, u32 length)
{
    CHECK_FATAL(!str, "str is null");
    CHECK_FATAL(start >= str->size, "index out of bounds");

    String* result = string_create();

    u32 end     = start + length;
    u32 str_len = string_len(str);
    if (end > str_len) {
        end = str_len;
    }

    u32 actual_len = end - start;

    if (actual_len > 0) { // Insert substring all at once
        const char* csrc = string_data_ptr(str) + start;
        genVec_insert_multi(result, 0, (const u8*)csrc, actual_len);
    }

    return result;
}


void string_print(const String* str)
{
    CHECK_FATAL(!str, "str is null");

    putchar('\"');
    for (u32 i = 0; i < str->size; i++) {
        putchar(((char*)str->data)[i]);
    }
    putchar('\"');
}


u32 cstr_len(const char* cstr)
{
    u32 len = 0;
    u32 i   = 0;

    while (cstr[i++] != '\0') {
        len++;
    }

    return len;
}


