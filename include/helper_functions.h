#pragma once

#include "String.h"
#include "gen_vector.h"
#include "hashmap.h"
#include "hashset.h"

#include <stdio.h>


#define cast(x) ((u8*)(&x))


// ================== VECTOR =====================

// INT
void vec_push_int(genVec* vec, int x) {
    genVec_push(vec, cast(x));
}

int vec_pop_int(genVec* vec) {
    int a;
    genVec_pop(vec, cast(a));
    return a;
}

int vec_get_int(genVec* vec, size_t i) {
    int a;
    genVec_get(vec, i, cast(a));
    return a;
}

// more int funcs....  // TODO: this can be a macro for basic types?



// STRING
void vec_push_str(genVec* vec, const char* cstr) {
    String str;
    string_create_onstack(&str, cstr);
    genVec_push(vec, cast(str));
}

const char* vec_pop_str(genVec* vec) {
    String str;
    genVec_pop(vec, cast(str));
    const char* popped = string_to_cstr(&str);
    string_destroy_fromstk(&str);
    return popped;
}

const char* vec_get_str(genVec* vec, size_t i) {
    String str;
    genVec_get(vec, i, cast(str));
    const char* cstr = string_to_cstr(&str);
    return cstr;
}



// ================== HASHMAP =====================


// STRING -> INT
void map_put_strToInt(hashmap* map, const char* key, int val) {
    String str;
    string_create_onstack(&str, key);
    hashmap_put(map, cast(str), cast(val));
}

int map_get_strToInt(hashmap* map, const char* key) {
    String str;
    string_create_onstack(&str, key);
    int a;
    hashmap_get(map, cast(str), cast(a));
    string_destroy_fromstk(&str);
    return a;
}

int map_has_strToInt(hashmap* map, const char* key) {
    String str;
    string_create_onstack(&str, key);
    int res = hashmap_has(map, cast(str));
    string_destroy_fromstk(&str);
    return res;
}




// ================== HASHSET =====================


// INT
void set_insert_int(hashset* set, int x) {
    hashset_insert(set, cast(x));
}

int set_has_int(hashset* set, int x) {
    return hashset_has(set, cast(x));
}

void set_remove_int(hashset* set, int x) {
    hashset_remove(set, cast(x));
}


// ======================= PRINT FUNCTIONS ============================

void int_print(const u8* elm) {
    printf("%d", *(int*)elm);
}

void str_print(const u8* elm) {
    String* str = (String*)elm;
    string_print(str);
}

