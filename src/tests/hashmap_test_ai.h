#include "hashmap.h"
#include "String.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper macros
#define TEST_START(name) printf("\n=== Testing %s ===\n", name)
#define TEST_PASS() printf("✓ PASSED\n")
#define TEST_FAIL(msg) printf("✗ FAILED: %s\n", msg)
#define ASSERT(cond, msg) if (!(cond)) { TEST_FAIL(msg); return 1; }

// Print functions
void print_int(const u8* elm) {
    printf("%d", *(int*)elm);
}

void print_str(const u8* elm) {
    printf("\"%s\"", (char*)elm);
}

void print_string_ptr(const u8* elm) {
    String* s = *(String**)elm;
    string_print(s);
}

// String copy/move/delete for when String is stored by value
void string_copy_val(u8* dest, const u8* src) {
    String* d = (String*)dest;
    String* s = (String*)src;
    string_copy(d, s);
}

void string_move_val(u8* dest, u8** src) {
    String* s = *(String**)src;
    memcpy(dest, s, sizeof(String));
    s->buffer.data = NULL;
    free(s);
    *src = NULL;
}

void string_del_val(u8* elm) {
    string_destroy_stk((String*)elm);
}

// String copy/move/delete for when String* is stored
void string_copy_ptr(u8* dest, const u8* src) {
    String* s = *(String**)src;
    String* d = malloc(sizeof(String));
    string_copy(d, s);
    *(String**)dest = d;
}

void string_move_ptr(u8* dest, u8** src) {
    *(String**)dest = *(String**)src;
    *src = NULL;
}

void string_del_ptr(u8* elm) {
    String* s = *(String**)elm;
    if (s) {
        string_destroy(s);
    }
}


// Test 1: Basic int -> int hashmap
int hashmap_test_basic(void)
{
    TEST_START("basic int->int operations");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
    
    ASSERT(hashmap_empty(map), "new map should be empty");
    ASSERT(hashmap_size(map) == 0, "new map size should be 0");
    
    // Insert some values
    int key1 = 10, val1 = 100;
    b8 result = hashmap_put(map, (u8*)&key1, (u8*)&val1);
    ASSERT(result == 0, "put new key should return 0");
    ASSERT(hashmap_size(map) == 1, "size should be 1");
    
    int key2 = 20, val2 = 200;
    hashmap_put(map, (u8*)&key2, (u8*)&val2);
    ASSERT(hashmap_size(map) == 2, "size should be 2");
    
    // Get values
    int out;
    result = hashmap_get(map, (u8*)&key1, (u8*)&out);
    ASSERT(result == 1, "get should find key");
    ASSERT(out == 100, "value should be 100");
    
    result = hashmap_get(map, (u8*)&key2, (u8*)&out);
    ASSERT(result == 1, "get should find key2");
    ASSERT(out == 200, "value should be 200");
    
    // Update existing key
    int val3 = 999;
    result = hashmap_put(map, (u8*)&key1, (u8*)&val3);
    ASSERT(result == 1, "put existing key should return 1");
    ASSERT(hashmap_size(map) == 2, "size should still be 2");
    
    hashmap_get(map, (u8*)&key1, (u8*)&out);
    ASSERT(out == 999, "value should be updated to 999");
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test 2: String key operations (cstr -> int)
int hashmap_test_string_key(void)
{
    TEST_START("string key operations");
    
    hashmap* map = hashmap_create(20, sizeof(int), NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
    
    // Insert with string keys
    char key1[] = "hello";
    int val1 = 42;
    hashmap_put(map, (u8*)key1, (u8*)&val1);
    
    char key2[] = "world";
    int val2 = 84;
    hashmap_put(map, (u8*)key2, (u8*)&val2);
    
    ASSERT(hashmap_size(map) == 2, "should have 2 entries");
    
    // Retrieve values
    int out;
    ASSERT(hashmap_get(map, (u8*)key1, (u8*)&out), "should find 'hello'");
    ASSERT(out == 42, "value for 'hello' should be 42");
    
    ASSERT(hashmap_get(map, (u8*)key2, (u8*)&out), "should find 'world'");
    ASSERT(out == 84, "value for 'world' should be 84");
    
    // Test has
    ASSERT(hashmap_has(map, (u8*)key1), "should have 'hello'");
    ASSERT(hashmap_has(map, (u8*)key2), "should have 'world'");
    
    char key3[] = "missing";
    ASSERT(!hashmap_has(map, (u8*)key3), "should not have 'missing'");
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test 3: Delete operations
int hashmap_test_delete(void)
{
    TEST_START("delete operations");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
    
    // Insert multiple values
    for (int i = 0; i < 10; i++) {
        int key = i * 10;
        int val = i * 100;
        hashmap_put(map, (u8*)&key, (u8*)&val);
    }
    
    ASSERT(hashmap_size(map) == 10, "should have 10 entries");
    
    // Delete a key
    int key = 50;
    b8 result = hashmap_del(map, (u8*)&key);
    ASSERT(result == 1, "delete should succeed");
    ASSERT(hashmap_size(map) == 9, "size should be 9");
    ASSERT(!hashmap_has(map, (u8*)&key), "key should not exist");
    
    // Try to delete non-existent key
    int missing = 999;
    result = hashmap_del(map, (u8*)&missing);
    ASSERT(result == 0, "delete non-existent should return 0");
    ASSERT(hashmap_size(map) == 9, "size should still be 9");
    
    // Delete all remaining
    for (int i = 0; i < 10; i++) {
        if (i == 5) continue; // already deleted
        int k = i * 10;
        hashmap_del(map, (u8*)&k);
    }
    
    ASSERT(hashmap_empty(map), "map should be empty");
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test 4: Get pointer
int hashmap_test_get_ptr(void)
{
    TEST_START("get pointer operations");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
    
    int key = 42, val = 100;
    hashmap_put(map, (u8*)&key, (u8*)&val);
    
    // Get pointer to value
    int* ptr = (int*)hashmap_get_ptr(map, (u8*)&key);
    ASSERT(ptr != NULL, "should get valid pointer");
    ASSERT(*ptr == 100, "dereferenced value should be 100");
    
    // Modify through pointer
    *ptr = 200;
    
    int out;
    hashmap_get(map, (u8*)&key, (u8*)&out);
    ASSERT(out == 200, "value should be modified to 200");
    
    // Non-existent key
    int missing = 999;
    ptr = (int*)hashmap_get_ptr(map, (u8*)&missing);
    ASSERT(ptr == NULL, "should return NULL for missing key");
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test 5: Resize and stress test
int hashmap_test_resize(void)
{
    TEST_START("resize and stress test");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
    
    u32 initial_capacity = hashmap_capacity(map);
    
    // Insert many elements to trigger resize
    for (int i = 0; i < 100; i++) {
        int key = i;
        int val = i * 10;
        hashmap_put(map, (u8*)&key, (u8*)&val);
    }
    
    ASSERT(hashmap_size(map) == 100, "should have 100 entries");
    ASSERT(hashmap_capacity(map) > initial_capacity, "capacity should have grown");
    
    // Verify all values are still accessible
    for (int i = 0; i < 100; i++) {
        int key = i;
        int out;
        ASSERT(hashmap_get(map, (u8*)&key, (u8*)&out), "should find key");
        ASSERT(out == i * 10, "value should match");
    }
    
    // Delete most elements to trigger shrink
    for (int i = 0; i < 90; i++) {
        int key = i;
        hashmap_del(map, (u8*)&key);
    }
    
    ASSERT(hashmap_size(map) == 10, "should have 10 entries left");
    
    // Verify remaining values
    for (int i = 90; i < 100; i++) {
        int key = i;
        int out;
        ASSERT(hashmap_get(map, (u8*)&key, (u8*)&out), "should find remaining key");
        ASSERT(out == i * 10, "value should still match");
    }
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test 6: String values with custom delete (int -> String*)
int hashmap_test_string_values(void)
{
    TEST_START("String* values with custom functions");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(String*), NULL, NULL,
                                   NULL, string_copy_ptr,
                                   NULL, string_move_ptr,
                                   NULL, string_del_ptr);
    
    // Insert String values
    int key1 = 1;
    String* str1 = string_from_cstr("first value");
    hashmap_put(map, (u8*)&key1, (u8*)&str1);
    
    int key2 = 2;
    String* str2 = string_from_cstr("second value");
    hashmap_put(map, (u8*)&key2, (u8*)&str2);
    
    ASSERT(hashmap_size(map) == 2, "should have 2 entries");
    
    // Get values
    String* out;
    ASSERT(hashmap_get(map, (u8*)&key1, (u8*)&out), "should find key1");
    ASSERT(string_equals_cstr(out, "first value"), "value should match");
    string_destroy(out); // We got a copy, must free it
    
    // Update value (old should be cleaned up)
    String* str3 = string_from_cstr("updated value");
    hashmap_put(map, (u8*)&key1, (u8*)&str3);
    
    ASSERT(hashmap_get(map, (u8*)&key1, (u8*)&out), "should find updated key1");
    ASSERT(string_equals_cstr(out, "updated value"), "value should be updated");
    string_destroy(out);
    
    // Cleanup originals (map made copies)
    string_destroy(str1);
    string_destroy(str2);
    string_destroy(str3);
    
    hashmap_destroy(map); // Should cleanup all String* values
    
    TEST_PASS();
    return 0;
}


// Test 7: Move semantics
int hashmap_test_move(void)
{
    TEST_START("move semantics");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(String*), NULL, NULL,
                                   NULL, string_copy_ptr,
                                   NULL, string_move_ptr,
                                   NULL, string_del_ptr);
    
    int key = 42;
    String* str = string_from_cstr("move me");
    
    // Move insert
    hashmap_put_move(map, (u8**)&key, (u8**)&str);
    
    ASSERT(str == NULL, "source should be nulled after move");
    ASSERT(hashmap_size(map) == 1, "should have 1 entry");
    
    // Verify value was moved
    String* out;
    int query_key = 42;
    ASSERT(hashmap_get(map, (u8*)&query_key, (u8*)&out), "should find key");
    ASSERT(string_equals_cstr(out, "move me"), "value should match");
    string_destroy(out);
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test 8: Collision handling
int hashmap_test_collisions(void)
{
    TEST_START("collision handling");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
    
    // Insert many values that might collide
    for (int i = 0; i < 50; i++) {
        int key = i;
        int val = i * 2;
        hashmap_put(map, (u8*)&key, (u8*)&val);
    }
    
    // Verify all are retrievable
    for (int i = 0; i < 50; i++) {
        int key = i;
        int out;
        ASSERT(hashmap_get(map, (u8*)&key, (u8*)&out), "should find all keys");
        ASSERT(out == i * 2, "all values should be correct");
    }
    
    // Delete some and verify others still work
    for (int i = 0; i < 25; i++) {
        int key = i;
        hashmap_del(map, (u8*)&key);
    }
    
    for (int i = 25; i < 50; i++) {
        int key = i;
        int out;
        ASSERT(hashmap_get(map, (u8*)&key, (u8*)&out), "remaining keys should work");
        ASSERT(out == i * 2, "remaining values should be correct");
    }
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test 9: Update operations
int hashmap_test_updates(void)
{
    TEST_START("update operations");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
    
    int key = 100;
    
    // Insert initial value
    int val1 = 1;
    b8 result = hashmap_put(map, (u8*)&key, (u8*)&val1);
    ASSERT(result == 0, "first put should return 0 (new)");
    
    // Update multiple times
    for (int i = 2; i <= 10; i++) {
        result = hashmap_put(map, (u8*)&key, (u8*)&i);
        ASSERT(result == 1, "update should return 1 (existing)");
        ASSERT(hashmap_size(map) == 1, "size should stay 1");
    }
    
    int out;
    hashmap_get(map, (u8*)&key, (u8*)&out);
    ASSERT(out == 10, "final value should be 10");
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test 10: Empty map operations
int hashmap_test_empty(void)
{
    TEST_START("empty map operations");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
    
    ASSERT(hashmap_empty(map), "new map should be empty");
    
    int key = 42, out;
    
    // Get from empty map
    ASSERT(!hashmap_get(map, (u8*)&key, (u8*)&out), "get from empty should fail");
    ASSERT(!hashmap_has(map, (u8*)&key), "has on empty should return false");
    ASSERT(hashmap_get_ptr(map, (u8*)&key) == NULL, "get_ptr on empty should return NULL");
    
    // Delete from empty map
    ASSERT(!hashmap_del(map, (u8*)&key), "delete from empty should return 0");
    
    hashmap_destroy(map);
    
    TEST_PASS();
    return 0;
}


// Test runner
int run(void)
{
    printf("\n╔════════════════════════════════════╗\n");
    printf("║     HASHMAP TEST SUITE             ║\n");
    printf("╚════════════════════════════════════╝\n");
    
    int failed = 0;
    
    failed += hashmap_test_basic();
    failed += hashmap_test_string_key();
    failed += hashmap_test_delete();
    failed += hashmap_test_get_ptr();
    failed += hashmap_test_resize();
    failed += hashmap_test_string_values();
    failed += hashmap_test_move();
    failed += hashmap_test_collisions();
    failed += hashmap_test_updates();
    failed += hashmap_test_empty();
    
    printf("\n╔════════════════════════════════════╗\n");
    if (failed == 0) {
        printf("║  ✓ ALL TESTS PASSED                ║\n");
    } else {
        printf("║  ✗ %d TEST(S) FAILED               ║\n", failed);
    }
    printf("╚════════════════════════════════════╝\n\n");
    
    return failed;
}


