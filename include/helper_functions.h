#pragma once

#include "String.h"
#include "gen_vector.h"
#include "hashmap.h"
#include "hashset.h"

#include <stdio.h>


#define cast(x) ((u8*)(&x))


// ================== VECTOR =====================

// INT
static inline void vec_push_int(genVec* vec, int x) {
    genVec_push(vec, cast(x));
}

static inline int vec_pop_int(genVec* vec) {
    int a;
    genVec_pop(vec, cast(a));
    return a;
}

static inline int vec_get_int(genVec* vec, size_t i) {
    int a;
    genVec_get(vec, i, cast(a));
    return a;
}

// more int funcs....  // TODO: this can be a macro for basic types?



// STRING
static inline void vec_push_str(genVec* vec, const char* cstr) {
    String str;
    string_create_onstack(&str, cstr);
    genVec_push(vec, cast(str));
    string_destroy_fromstk(&str);
}

static inline const char* vec_pop_str(genVec* vec) {
    String str;                                    
    genVec_pop(vec, cast(str));             // WARNING: 
    const char* popped = string_to_cstr(&str);  // TODO: this is so fucking wrong
    return popped;
}

static inline const char* vec_get_str(genVec* vec, size_t i) {
    String str;
    genVec_get(vec, i, cast(str));
    const char* cstr = string_to_cstr(&str);
    return cstr;
}



// ================== HASHMAP =====================


// STRING -> INT
static inline void map_put_strToInt(hashmap* map, const char* key, int val) {
    String str;
    string_create_onstack(&str, key);
    hashmap_put(map, cast(str), cast(val));
}

static inline int map_get_strToInt(hashmap* map, const char* key) {
    String str;
    string_create_onstack(&str, key);
    int a;
    hashmap_get(map, cast(str), cast(a));
    string_destroy_fromstk(&str);
    return a;
}

static inline int map_has_strToInt(hashmap* map, const char* key) {
    String str;
    string_create_onstack(&str, key);
    int res = hashmap_has(map, cast(str));
    string_destroy_fromstk(&str);
    return res;
}




// ================== HASHSET =====================


// INT
static inline void set_insert_int(hashset* set, int x) {
    hashset_insert(set, cast(x));
}

static inline int set_has_int(hashset* set, int x) {
    return hashset_has(set, cast(x));
}

static inline void set_remove_int(hashset* set, int x) {
    hashset_remove(set, cast(x));
}


// ======================= PRINT FUNCTIONS ============================

static inline void int_print(const u8* elm) {
    printf("%d", *(int*)elm);
}

static inline void str_print(const u8* elm) {
    String* str = (String*)elm;
    string_print(str);
}



static inline u8 int_cmp(const u8* a, const u8* b) // 1 if a > b, 0 if a < b, 255 if a = b
{
    int x = *(int*) a;
    int y = *(int*) b;

    if (x > y) { return 1; }
    else if (x < y) { return 0; }
    else { return -1; }
}

static inline String* int_to_str(const u8* elm)
{
    int num = *(int*)elm;
    String* str = string_create();
    
    if (num == 0) {
        string_append_char(str, '0');
        return str;
    }
    
    int is_negative = 0;
    if (num < 0) {
        is_negative = 1;
        num = -num;  // Make positive
    }
    
    // Build digits in reverse order
    String* temp = string_create();
    while (num > 0) {
        int digit = num % 10;
        string_append_char(temp, '0' + digit);  // Fixed: '0' + digit, not digit - '0'
        num /= 10;
    }
    
    // Add negative sign if needed
    if (is_negative) {
        string_append_char(str, '-');
    }
    
    // Reverse the digits
    size_t len = string_len(temp);
    for (int i = len - 1; i >= 0; i--) {
        string_append_char(str, string_at(temp, i));
    }
    
    string_destroy(temp);
    return str;
}

