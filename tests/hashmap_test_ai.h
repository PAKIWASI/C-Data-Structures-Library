#pragma once

#include "String.h"
#include "common.h"
#include "hashmap.h"
#include "helpers.h"
#include "str_setup.h"
#include <stdio.h>
#include <assert.h>


// ============================================================================
// TEST 1: Basic int->int operations (copy semantics)
// ============================================================================
int hashmap_test_basic_int(void)
{
    printf("\n=== TEST 1: Basic int->int operations ===\n");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL, 
                                  NULL, NULL, NULL, NULL, NULL, NULL);
    
    // Insert some key-value pairs
    for (int i = 0; i < 20; i++) {
        int val = i * 10;
        b8 existed = hashmap_put(map, cast(i), 0, cast(val), 0);
        assert(existed == 0); // Should be new insertions
    }
    
    printf("After inserting 20 elements:\n");
    printf("Size: %u, Capacity: %u\n", hashmap_size(map), hashmap_capacity(map));
    
    // Test retrieval
    for (int i = 0; i < 20; i++) {
        int val;
        b8 found = hashmap_get(map, cast(i), cast(val));
        assert(found == 1);
        assert(val == i * 10);
    }
    printf("✓ All 20 elements retrieved correctly\n");
    
    // Test update
    int key = 5;
    int new_val = 999;
    b8 existed = hashmap_put(map, cast(key), 0, cast(new_val), 0);
    assert(existed == 1); // Should be an update
    
    int retrieved;
    hashmap_get(map, cast(key), cast(retrieved));
    assert(retrieved == 999);
    printf("✓ Update operation works\n");
    
    // Test has
    assert(hashmap_has(map, cast(key)) == 1);
    int nonexistent = 100;
    assert(hashmap_has(map, cast(nonexistent)) == 0);
    printf("✓ Has operation works\n");
    
    // Test deletion
    b8 deleted = hashmap_del(map, cast(key), NULL);
    assert(deleted == 1);
    assert(hashmap_has(map, cast(key)) == 0);
    printf("✓ Deletion works\n");
    
    printf("Final size: %u\n", hashmap_size(map));
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 2: int->String VALUE (copy semantics)
// ============================================================================
int hashmap_test_string_val_copy(void)
{
    printf("\n=== TEST 2: int->String VALUE (copy semantics) ===\n");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(String), NULL, NULL, 
                                  NULL, str_copy, NULL, str_move, NULL, str_del);
    
    // Insert strings by value
    for (int i = 0; i < 10; i++) {
        String str;
        char buf[50];
        snprintf(buf, sizeof(buf), "String_%d", i);
        string_create_stk(&str, buf);
        
        hashmap_put(map, cast(i), 0, cast(str), 0);
        string_destroy_stk(&str);
    }
    
    printf("Inserted 10 strings (by value)\n");
    hashmap_print(map, int_print, str_print);
    
    // Verify retrieval
    int key = 5;
    String retrieved = {0};
    b8 found = hashmap_get(map, cast(key), cast(retrieved));
    assert(found == 1);
    
    printf("Retrieved: ");
    string_print(&retrieved);
    printf("\n");
    assert(string_equals_cstr(&retrieved, "String_5"));
    
    string_destroy_stk(&retrieved);
    
    // Test updating with different string
    String new_str;
    string_create_stk(&new_str, "UPDATED_STRING");
    hashmap_put(map, cast(key), 0, cast(new_str), 0);
    string_destroy_stk(&new_str);
    
    String check = {0};
    hashmap_get(map, cast(key), cast(check));
    assert(string_equals_cstr(&check, "UPDATED_STRING"));
    printf("✓ Update with new string works\n");
    string_destroy_stk(&check);
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 3: int->String VALUE (move semantics)
// ============================================================================
int hashmap_test_string_val_move(void)
{
    printf("\n=== TEST 3: int->String VALUE (move semantics) ===\n");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(String), NULL, NULL,
                                  NULL, str_copy, NULL, str_move, NULL, str_del);
    
    // Insert with move - String objects on heap
    for (int i = 0; i < 5; i++) {
        char buf[50];
        snprintf(buf, sizeof(buf), "MovedString_%d", i);
        String* str = string_from_cstr(buf);
        
        // For move, pass address of the heap String* pointer
        hashmap_put(map, cast(i), 0, (u8*)&str, 1);
        assert(str == NULL); // Should be nulled after move
    }
    
    printf("Inserted 5 strings using move semantics\n");
    hashmap_print(map, int_print, str_print);
    
    // Update with move
    int key = 2;
    String* new_str = string_from_cstr("MOVED_UPDATE");
    hashmap_put(map, cast(key), 0, (u8*)&new_str, 1);
    assert(new_str == NULL);
    
    String check = {0};
    hashmap_get(map, cast(key), cast(check));
    assert(string_equals_cstr(&check, "MOVED_UPDATE"));
    printf("✓ Move update works\n");
    string_destroy_stk(&check);
    
    // Test del with output
    String deleted_val = {0};
    b8 found = hashmap_del(map, cast(key), cast(deleted_val));
    assert(found == 1);
    printf("Deleted value: ");
    string_print(&deleted_val);
    printf("\n");
    string_destroy_stk(&deleted_val);
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 4: String VALUE key -> int value (custom hash for value)
// ============================================================================
int hashmap_test_string_val_key(void)
{
    printf("\n=== TEST 4: String VALUE key -> int (custom hash) ===\n");
    
    hashmap* map = hashmap_create(sizeof(String), sizeof(int), 
                                  murmurhash3_str_val, str_cmp_val,
                                  str_copy, NULL, str_move, NULL, 
                                  str_del, NULL);
    
    // Insert with string keys (by value)
    const char* keys[] = {"apple", "banana", "cherry", "date", "elderberry"};
    for (int i = 0; i < 5; i++) {
        String key;
        string_create_stk(&key, keys[i]);
        int val = i * 100;
        
        hashmap_put(map, cast(key), 0, cast(val), 0);
        string_destroy_stk(&key);
    }
    
    printf("Inserted 5 fruit->value pairs\n");
    hashmap_print(map, str_print, int_print);
    
    // Lookup
    String search_key;
    string_create_stk(&search_key, "cherry");
    int val;
    b8 found = hashmap_get(map, cast(search_key), cast(val));
    assert(found == 1);
    assert(val == 200);
    printf("✓ Found 'cherry' -> %d\n", val);
    
    // Test has
    assert(hashmap_has(map, cast(search_key)) == 1);
    string_destroy_stk(&search_key);
    
    String nonexistent;
    string_create_stk(&nonexistent, "mango");
    assert(hashmap_has(map, cast(nonexistent)) == 0);
    string_destroy_stk(&nonexistent);
    printf("✓ Has works for string keys\n");
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 5: String VALUE -> String VALUE (both copy)
// ============================================================================
int hashmap_test_string_val_both_copy(void)
{
    printf("\n=== TEST 5: String VAL->String VAL (both copy) ===\n");
    
    hashmap* map = hashmap_create(sizeof(String), sizeof(String),
                                  murmurhash3_str_val, str_cmp_val,
                                  str_copy, str_copy, str_move, str_move,
                                  str_del, str_del);
    
    // Insert String objects (not pointers)
    const char* keys[] = {"red", "green", "blue"};
    const char* vals[] = {"#FF0000", "#00FF00", "#0000FF"};
    
    for (int i = 0; i < 3; i++) {
        String key;
        String val;
        string_create_stk(&key, keys[i]);
        string_create_stk(&val, vals[i]);
        
        // Copy semantics - passing the String objects themselves
        hashmap_put(map, cast(key), 0, cast(val), 0);
        
        string_destroy_stk(&key);
        string_destroy_stk(&val);
    }
    
    printf("Inserted 3 color mappings with copy semantics\n");
    hashmap_print(map, str_print, str_print);
    
    // Update existing key with copy
    String update_key;
    String update_val;
    string_create_stk(&update_key, "red");
    string_create_stk(&update_val, "#FFFFFF");
    
    b8 existed = hashmap_put(map, cast(update_key), 0, cast(update_val), 0);
    assert(existed == 1); // Should be an update
    
    string_destroy_stk(&update_key);
    string_destroy_stk(&update_val);
    
    // Lookup and verify
    String lookup_key;
    string_create_stk(&lookup_key, "red");
    String result = {0};
    hashmap_get(map, cast(lookup_key), cast(result));
    assert(string_equals_cstr(&result, "#FFFFFF"));
    printf("✓ Update works for String keys\n");
    string_destroy_stk(&lookup_key);
    string_destroy_stk(&result);
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 6: String VALUE -> String VALUE (both move)
// ============================================================================
int hashmap_test_string_val_both_move(void)
{
    printf("\n=== TEST 6: String VAL->String VAL (both move) ===\n");
    
    hashmap* map = hashmap_create(sizeof(String), sizeof(String),
                                  murmurhash3_str_val, str_cmp_val,
                                  str_copy, str_copy, str_move, str_move,
                                  str_del, str_del);
    
    // Insert with both key and value moved
    const char* keys[] = {"red", "green", "blue"};
    const char* vals[] = {"#FF0000", "#00FF00", "#0000FF"};
    
    for (int i = 0; i < 3; i++) {
        String* key = string_from_cstr(keys[i]);
        String* val = string_from_cstr(vals[i]);
        
        // For move, pass address of String* pointers
        hashmap_put(map, (u8*)&key, 1, (u8*)&val, 1);
        assert(key == NULL);
        assert(val == NULL);
    }
    
    printf("Inserted 3 color mappings with move semantics\n");
    hashmap_print(map, str_print, str_print);
    
    // Update existing key with move
    String* update_key = string_from_cstr("red");
    String* update_val = string_from_cstr("#FFFFFF");
    
    b8 existed = hashmap_put(map, (u8*)&update_key, 1, (u8*)&update_val, 1);
    assert(existed == 1); // Should be an update
    assert(update_key == NULL); // Key should be freed since it existed
    assert(update_val == NULL);
    
    String lookup_key;
    string_create_stk(&lookup_key, "red");
    String result = {0};
    hashmap_get(map, cast(lookup_key), cast(result));
    assert(string_equals_cstr(&result, "#FFFFFF"));
    printf("✓ Move update works for existing key\n");
    string_destroy_stk(&lookup_key);
    string_destroy_stk(&result);
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 7: int->String* POINTER (copy semantics)
// ============================================================================
int hashmap_test_string_ptr_copy(void)
{
    printf("\n=== TEST 7: int->String* POINTER (copy semantics) ===\n");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(String*), NULL, NULL,
                                  NULL, str_copy_ptr, NULL, str_move_ptr, 
                                  NULL, str_del_ptr);
    
    // Insert String pointers
    for (int i = 0; i < 5; i++) {
        char buf[50];
        snprintf(buf, sizeof(buf), "PtrString_%d", i);
        String* str = string_from_cstr(buf);
        
        // For pointers, pass address of the pointer (which is String**)
        hashmap_put(map, cast(i), 0, (u8*)&str, 0);
        
        // We can still use the original since it was copied
        string_destroy(str);
    }
    
    printf("Inserted 5 String pointers\n");
    hashmap_print(map, int_print, str_print_ptr);
    
    // Retrieve
    int key = 2;
    String* retrieved = NULL;
    hashmap_get(map, cast(key), (u8*)&retrieved);
    assert(retrieved != NULL);
    printf("Retrieved: ");
    string_print(retrieved);
    printf("\n");
    
    // Don't destroy retrieved - it's owned by the map
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 8: int->String* POINTER (move semantics)
// ============================================================================
int hashmap_test_string_ptr_move(void)
{
    printf("\n=== TEST 8: int->String* POINTER (move semantics) ===\n");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(String*), NULL, NULL,
                                  NULL, str_copy_ptr, NULL, str_move_ptr,
                                  NULL, str_del_ptr);
    
    // Insert with move - String* pointers
    for (int i = 0; i < 5; i++) {
        char buf[50];
        snprintf(buf, sizeof(buf), "MovedPtr_%d", i);
        String* str = string_from_cstr(buf);
        
        // For move of pointer, pass String** (address of String* pointer)
        hashmap_put(map, cast(i), 0, (u8*)&str, 1);
        assert(str == NULL); // Should be nulled
    }
    
    printf("Inserted 5 String* with move semantics\n");
    hashmap_print(map, int_print, str_print_ptr);
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 9: String* POINTER key -> int value
// ============================================================================
int hashmap_test_string_ptr_key(void)
{
    printf("\n=== TEST 9: String* POINTER key -> int ===\n");
    
    hashmap* map = hashmap_create(sizeof(String*), sizeof(int),
                                  murmurhash3_str_ptr, str_cmp_ptr,
                                  str_copy_ptr, NULL, str_move_ptr, NULL,
                                  str_del_ptr, NULL);
    
    // Insert with String* keys
    const char* keys[] = {"alpha", "beta", "gamma", "delta"};
    for (int i = 0; i < 4; i++) {
        String* key = string_from_cstr(keys[i]);
        int val = i * 50;
        
        hashmap_put(map, (u8*)&key, 0, cast(val), 0);
        
        // Original key is copied, so we can free it
        string_destroy(key);
    }
    
    printf("Inserted 4 String* keys\n");
    hashmap_print(map, str_print_ptr, int_print);
    
    // Lookup
    String* search_key = string_from_cstr("gamma");
    int val;
    b8 found = hashmap_get(map, (u8*)&search_key, cast(val));
    assert(found == 1);
    assert(val == 100);
    printf("✓ Found 'gamma' -> %d\n", val);
    
    string_destroy(search_key);
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 10: Stress test - many insertions and deletions
// ============================================================================
int hashmap_test_stress(void)
{
    printf("\n=== TEST 10: Stress test (1000 operations) ===\n");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                  NULL, NULL, NULL, NULL, NULL, NULL);
    
    // Insert 1000 elements
    for (int i = 0; i < 1000; i++) {
        int val = i * 7;
        hashmap_put(map, cast(i), 0, cast(val), 0);
    }
    printf("Inserted 1000 elements\n");
    printf("Size: %u, Capacity: %u\n", hashmap_size(map), hashmap_capacity(map));
    
    // Verify all exist
    for (int i = 0; i < 1000; i++) {
        assert(hashmap_has(map, cast(i)) == 1);
    }
    printf("✓ All 1000 elements verified\n");
    
    // Delete every other element
    for (int i = 0; i < 1000; i += 2) {
        b8 deleted = hashmap_del(map, cast(i), NULL);
        assert(deleted == 1);
    }
    printf("Deleted 500 elements\n");
    printf("Size: %u, Capacity: %u\n", hashmap_size(map), hashmap_capacity(map));
    
    // Verify deletions
    for (int i = 0; i < 1000; i++) {
        if (i % 2 == 0) {
            assert(hashmap_has(map, cast(i)) == 0);
        } else {
            assert(hashmap_has(map, cast(i)) == 1);
        }
    }
    printf("✓ Deletions verified\n");
    
    // Re-insert deleted elements
    for (int i = 0; i < 1000; i += 2) {
        int val = i * 13;
        hashmap_put(map, cast(i), 0, cast(val), 0);
    }
    printf("Re-inserted 500 elements\n");
    printf("Final size: %u, Capacity: %u\n", hashmap_size(map), hashmap_capacity(map));
    assert(hashmap_size(map) == 1000);
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 11: Edge cases
// ============================================================================
int hashmap_test_edge_cases(void)
{
    printf("\n=== TEST 11: Edge cases ===\n");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(int), NULL, NULL,
                                  NULL, NULL, NULL, NULL, NULL, NULL);
    
    // Empty map operations
    int key = 42;
    int val;
    assert(hashmap_get(map, cast(key), cast(val)) == 0);
    assert(hashmap_has(map, cast(key)) == 0);
    assert(hashmap_del(map, cast(key), NULL) == 0);
    assert(hashmap_empty(map) == 1);
    printf("✓ Empty map operations work\n");
    
    // Single element
    val = 100;
    hashmap_put(map, cast(key), 0, cast(val), 0);
    assert(hashmap_size(map) == 1);
    assert(hashmap_has(map, cast(key)) == 1);
    
    int retrieved;
    hashmap_get(map, cast(key), cast(retrieved));
    assert(retrieved == 100);
    printf("✓ Single element operations work\n");
    
    // Delete and verify empty again
    hashmap_del(map, cast(key), NULL);
    assert(hashmap_empty(map) == 1);
    printf("✓ Delete to empty works\n");
    
    // Multiple updates to same key
    for (int i = 0; i < 10; i++) {
        int v = i * 2;
        hashmap_put(map, cast(key), 0, cast(v), 0);
    }
    assert(hashmap_size(map) == 1);
    hashmap_get(map, cast(key), cast(retrieved));
    assert(retrieved == 18);
    printf("✓ Multiple updates work\n");
    
    // Get pointer and modify
    int* ptr = (int*)hashmap_get_ptr(map, cast(key));
    assert(ptr != NULL);
    *ptr = 999;
    hashmap_get(map, cast(key), cast(retrieved));
    assert(retrieved == 999);
    printf("✓ Get pointer and modify works\n");
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// TEST 12: Mixed copy and move operations
// ============================================================================
int hashmap_test_mixed_semantics(void)
{
    printf("\n=== TEST 12: Mixed copy/move semantics ===\n");
    
    hashmap* map = hashmap_create(sizeof(int), sizeof(String), NULL, NULL,
                                  NULL, str_copy, NULL, str_move, NULL, str_del);
    
    // Copy insert
    int key1 = 1;
    String str1;
    string_create_stk(&str1, "copy_insert");
    hashmap_put(map, cast(key1), 0, cast(str1), 0);
    string_destroy_stk(&str1);
    
    // Move insert
    int key2 = 2;
    String* str2 = string_from_cstr("move_insert");
    hashmap_put(map, cast(key2), 0, (u8*)&str2, 1);
    assert(str2 == NULL);
    
    // Copy update
    String update1;
    string_create_stk(&update1, "copy_update");
    hashmap_put(map, cast(key1), 0, cast(update1), 0);
    string_destroy_stk(&update1);
    
    // Move update
    String* update2 = string_from_cstr("move_update");
    hashmap_put(map, cast(key2), 0, (u8*)&update2, 1);
    assert(update2 == NULL);
    
    printf("After mixed operations:\n");
    hashmap_print(map, int_print, str_print);
    
    // Verify
    String result1 = {0};
    hashmap_get(map, cast(key1), cast(result1));
    assert(string_equals_cstr(&result1, "copy_update"));
    string_destroy_stk(&result1);
    
    String result2 = {0};
    hashmap_get(map, cast(key2), cast(result2));
    assert(string_equals_cstr(&result2, "move_update"));
    string_destroy_stk(&result2);
    
    printf("✓ Mixed semantics work correctly\n");
    
    hashmap_destroy(map);
    printf("✓ Test passed!\n");
    return 0;
}

// ============================================================================
// RUN ALL TESTS
// ============================================================================
int run(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      COMPREHENSIVE HASHMAP TEST SUITE                 ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    hashmap_test_basic_int();
    hashmap_test_string_val_copy();
    hashmap_test_string_val_move();
    hashmap_test_string_val_key();
    hashmap_test_string_val_both_copy();
    hashmap_test_string_val_both_move();
    hashmap_test_string_ptr_copy();
    hashmap_test_string_ptr_move();
    hashmap_test_string_ptr_key();
    hashmap_test_stress();
    hashmap_test_edge_cases();
    hashmap_test_mixed_semantics();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      ALL TESTS PASSED! ✓                              ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 0;
}


