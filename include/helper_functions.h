#pragma once

#include "String.h"
#include "gen_vector.h"
#include "hashmap.h"
#include "hashset.h"

#include <stddef.h>
#include <stdio.h>



// ================== VECTOR MACROS =====================

// Generic vector operations for basic types

#define VEC_POP(vec, type) ({ \
    type _tmp; \
    genVec_pop((vec), cast(_tmp)); \
    _tmp; \
})

#define VEC_PUSH(vec, val) genVec_push((vec), cast(val))

#define VEC_GET(vec, idx) genVec_get_ptr((vec), (idx)); 

#define VEC_REPLACE(vec, idx, val) genVec_replace((vec), (idx), cast(val))

#define VEC_INSERT(vec, idx, val) genVec_insert((vec), (idx), cast(val))

#define VEC_PUSH_MULTI(vec, type, ...) \
    do { \
        type temp[] = {__VA_ARGS__}; \
        for (u32 i = 0; i < sizeof(temp)/sizeof(temp[0]); i++) { \
            genVec_push(vec, (u8*)&temp[i]); \
        } \
    } while(0)


// ================== SPECIFIC TYPE HELPERS =====================

// INT
static inline void vec_push_int(genVec* vec, int x) {
    VEC_PUSH(vec, x);
}

#define vec_push_ints(vec, ...) VEC_PUSH_MULTI(vec, int, __VA_ARGS__)

static inline int vec_pop_int(genVec* vec) {
    return VEC_POP(vec, int);
}

static inline int vec_get_int(genVec* vec, u32 i) {
    return *(int*)VEC_GET(vec, i);
}

static inline void vec_replace_int(genVec* vec, u32 i, int x) {
    VEC_REPLACE(vec, i, x);
}

static inline void vec_insert_int(genVec* vec, u32 i, int x) {
    VEC_INSERT(vec, i, x);
}

// FLOAT
static inline void vec_push_float(genVec* vec, float x) {
    VEC_PUSH(vec, x);
}

static inline float vec_pop_float(genVec* vec) {
    return VEC_POP(vec, float);
}

static inline float vec_get_float(genVec* vec, u32 i) {
    return *(float*)VEC_GET(vec, i);
}

static inline void vec_replace_float(genVec* vec, u32 i, float x) {
    VEC_REPLACE(vec, i, x);
}

static inline void vec_insert_float(genVec* vec, u32 i, float x) {
    VEC_INSERT(vec, i, x);
}

// DOUBLE
static inline void vec_push_double(genVec* vec, double x) {
    VEC_PUSH(vec, x);
}

static inline double vec_pop_double(genVec* vec) {
    return VEC_POP(vec, double);
}

static inline double vec_get_double(genVec* vec, u32 i) {
    return *(double*)VEC_GET(vec, i);
}

static inline void vec_replace_double(genVec* vec, u32 i, double x) {
    VEC_REPLACE(vec, i, x);
}
static inline void vec_insert_double(genVec* vec, u32 i, double x) {
    VEC_INSERT(vec, i, x);
}

// CHAR
static inline void vec_push_char(genVec* vec, char x) {
    VEC_PUSH(vec, x);
}

static inline char vec_pop_char(genVec* vec) {
    return VEC_POP(vec, char);
}

static inline char vec_get_char(genVec* vec, u32 i) {
    return *(char*)VEC_GET(vec, i);
}

static inline void vec_replace_char(genVec* vec, u32 i, char x) {
    VEC_REPLACE(vec, i, x);
}

static inline void vec_insert_char(genVec* vec, u32 i, char x) {
    VEC_INSERT(vec, i, x);
}


// STRING
static inline void vec_push_cstr(genVec* vec, const char* cstr) {
    String str;
    string_create_onstk(&str, cstr);
    genVec_push(vec, cast(str));
    // if destroy from stk : buffer gets deleted
}

// have to call delete on the string
static inline String* vec_pop_str(genVec* vec) {
    String* str = string_create();
    genVec_pop(vec, (u8*)str);
    return str;
}

// getting pointer to the raw data
static inline const char* vec_get_cstr(genVec* vec, u32 i) {
    return string_to_cstr_ptr((String*)genVec_get_ptr(vec, i)); 
}

static inline String* vec_get_str(genVec* vec, u32 i) {
    String* str = string_create();
    genVec_get(vec, i, (u8*)str);
    return str;
}

static inline void vec_replace_cstr(genVec* vec, u32 i, const char* cstr) {
    String* str = string_from_cstr(cstr);
    genVec_replace(vec, i, (u8*)str);
}

// inserting String* is a 1-liner
static inline void vec_insert_cstr(genVec* vec, u32 i, const char* cstr) {
    String* str = string_from_cstr(cstr);
    genVec_replace(vec, i, (u8*)str);
}

// ================== HASHMAP MACROS =====================

#define MAP_PUT(map, key, val) hashmap_put((map), cast(key), cast(val))

#define MAP_GET(map, key, type) ({ \
    type _tmp; \
    hashmap_get((map), cast(key), cast(_tmp)); \
    _tmp; \
})

#define MAP_HAS(map, key) hashmap_has((map), cast(key))

#define MAP_DEL(map, key) hashmap_del((map), cast(key))


// ================== HASHMAP TYPE HELPERS =====================

// INT -> INT
static inline void map_put_intToInt(hashmap* map, int key, int val) {
    MAP_PUT(map, key, val);
}

static inline int map_get_intToInt(hashmap* map, int key) {
    return MAP_GET(map, key, int);
}

static inline int map_has_intToInt(hashmap* map, int key) {
    return MAP_HAS(map, key);
}

static inline void map_del_intToInt(hashmap* map, int key) {
    MAP_DEL(map, key);
}

// STRING -> INT
static inline void map_put_strToInt(hashmap* map, const char* key, int val) {
    String str;
    string_create_onstk(&str, key);
    if (hashmap_put(map, cast(str), cast(val))) { // if found
        string_destroy_fromstk(&str);
    }
}

static inline int map_get_strToInt(hashmap* map, const char* key) {
    String str;
    string_create_onstk(&str, key);
    int a;
    hashmap_get(map, cast(str), cast(a));
    string_destroy_fromstk(&str);
    return a;
}

static inline int map_has_strToInt(hashmap* map, const char* key) {
    String str;
    string_create_onstk(&str, key);
    int res = hashmap_has(map, cast(str));
    string_destroy_fromstk(&str);
    return res;
}

static inline void map_del_strToInt(hashmap* map, const char* key) {
    String str;
    string_create_onstk(&str, key);
    hashmap_del(map, cast(str));
    string_destroy_fromstk(&str);
}

// STRING -> STRING
static inline void map_put_strToStr(hashmap* map, const char* key, const char* val) {
    String kstr;
    String vstr;
    string_create_onstk(&kstr, key);
    string_create_onstk(&vstr, val);
    if (hashmap_put(map, cast(kstr), cast(vstr))) { // if found, key is not copied, need to free it
        string_destroy_fromstk(&kstr);
    }
}

static inline String* map_get_strToStr(hashmap* map, const char* key) {
    String kstr;
    string_create_onstk(&kstr, key);
    String* vstr = string_create();
    if (!hashmap_get(map, cast(kstr), (u8*)vstr)) {
        string_destroy(vstr); 
        vstr = NULL; 
    }
    string_destroy_fromstk(&kstr);
    return vstr;
}

static inline u8* map_get_ptr_strtoStr(hashmap* map, const char* key) {
    String kstr;
    string_create_onstk(&kstr, key);
    u8* res = hashmap_get_ptr(map, cast(kstr));
    string_destroy_fromstk(&kstr);
    return res;
}

static inline void map_del_strToStr(hashmap* map, const char* key) {
    String kstr;
    string_create_onstk(&kstr, key);
    hashmap_del(map, cast(kstr));
    string_destroy_fromstk(&kstr);
}

static inline u8 map_has_strToStr(hashmap* map, const char* key) {
    String kstr;
    string_create_onstk(&kstr, key);
    u8 res = hashmap_has(map, cast(kstr));
    string_destroy_fromstk(&kstr);
    return res;
}


// ================== HASHSET MACROS =====================

#define SET_INSERT(set, val) hashset_insert((set), cast(val))

#define SET_HAS(set, val) hashset_has((set), cast(val))

#define SET_REMOVE(set, val) hashset_remove((set), cast(val))


// ================== HASHSET TYPE HELPERS =====================

// INT
static inline void set_insert_int(hashset* set, int x) {
    SET_INSERT(set, x);
}

static inline int set_has_int(hashset* set, int x) {
    return SET_HAS(set, x);
}

static inline void set_remove_int(hashset* set, int x) {
    SET_REMOVE(set, x);
}

// CHAR
static inline void set_insert_char(hashset* set, char x) {
    SET_INSERT(set, x);
}

static inline int set_has_char(hashset* set, char x) {
    return SET_HAS(set, x);
}

static inline void set_remove_char(hashset* set, char x) {
    SET_REMOVE(set, x);
}

// STRING
static inline void set_insert_str(hashset* set, const char* cstr) {
    String str;
    string_create_onstk(&str, cstr);
    hashset_insert(set, cast(str));
}

static inline int set_has_str(hashset* set, const char* cstr) {
    String str;
    string_create_onstk(&str, cstr);
    int res = hashset_has(set, cast(str));
    string_destroy_fromstk(&str);
    return res;
}

static inline void set_remove_str(hashset* set, const char* cstr) {
    String str;
    string_create_onstk(&str, cstr);
    hashset_remove(set, cast(str));
    string_destroy_fromstk(&str);
}


// ======================= PRINT FUNCTIONS ============================

static inline void int_print(const u8* elm) {
    printf("%d", *(int*)elm);
}

static inline void float_print(const u8* elm) {
    printf("%.2f", *(float*)elm);
}

static inline void double_print(const u8* elm) {
    printf("%.2lf", *(double*)elm);
}

static inline void char_print(const u8* elm) {
    printf("%c", *(char*)elm);
}

static inline void str_print(const u8* elm) {
    String* str = (String*)elm;
    string_print(str);
}

// Print function for vector of vectors
static inline void vecvecint_print(const u8* elm) 
{
    genVec* vec = (genVec*)elm;

    printf("[ ");
    for (u32 i = 0; i < genVec_size(vec); i++) {
        const u8* element = genVec_get_ptr(vec, i);
        int_print(element);
        printf(" ");
    }
    printf("]\n");
}

// ======================= COMPARE FUNCTIONS ============================

// Returns: 1 if a > b, 0 if a < b, 255 (-1) if a == b
static inline u8 int_cmp(const u8* a, const u8* b) {
    int x = *(int*)a;
    int y = *(int*)b;

    if (x > y) { return 1; } 
    else if (x < y) { return 0; } 
    else { return 255; }
}

static inline u8 float_cmp(const u8* a, const u8* b) {
    float x = *(float*)a;
    float y = *(float*)b;

    if (x > y) { return 1; }
    else if (x < y) { return 0; }
    else { return 255; }
}

static inline u8 double_cmp(const u8* a, const u8* b) {
    double x = *(double*)a;
    double y = *(double*)b;

    if (x > y) { return 1; }
    else if (x < y) { return 0; }
    else { return 255; }
}

static inline u8 char_cmp(const u8* a, const u8* b) {
    char x = *(char*)a;
    char y = *(char*)b;

    if (x > y) { return 1; }
    else if (x < y) { return 0; } 
    else { return 255; }
}


// ======================= TO_STRING FUNCTIONS ============================

static inline String* int_to_str(const u8* elm) {
    int num = *(int*)elm;
    String* str = string_create();
    
    if (num == 0) {
        string_append_char(str, '0');
        return str;
    }
    
    int is_negative = 0;
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    String* temp = string_create();
    while (num > 0) {
        int digit = num % 10;
        string_append_char(temp, '0' + digit);
        num /= 10;
    }
    
    if (is_negative) {
        string_append_char(str, '-');
    }
    
    u32 len = string_len(temp);
    for (u32 i = len - 1; i >= 0; i--) {
        string_append_char(str, string_char_at(temp, i));
    }
    
    string_destroy(temp);
    return str;
}



