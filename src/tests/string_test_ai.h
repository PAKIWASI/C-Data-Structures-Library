#include "String.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper macro for test reporting
#define TEST_START(name) printf("\n=== Testing %s ===\n", name)
#define TEST_PASS() printf("✓ PASSED\n")
#define TEST_FAIL(msg) printf("✗ FAILED: %s\n", msg)
#define ASSERT(cond, msg) if (!(cond)) { TEST_FAIL(msg); return 1; }


// Test 1: Basic creation and destruction
int string_test_create_destroy(void)
{
    TEST_START("create and destroy");
    
    // Test heap creation
    String* str1 = string_create();
    ASSERT(str1 != NULL, "heap creation failed");
    ASSERT(string_len(str1) == 0, "initial length not 0");
    ASSERT(string_empty(str1), "empty check failed");
    string_destroy(str1);
    
    // Test from cstr
    String* str2 = string_from_cstr("hello world");
    ASSERT(str2 != NULL, "from_cstr creation failed");
    ASSERT(string_len(str2) == 11, "length incorrect");
    ASSERT(!string_empty(str2), "should not be empty");
    string_destroy(str2);
    
    // Test stack creation
    String str3;
    string_create_stk(&str3, "stack string");
    ASSERT(string_len(&str3) == 12, "stack string length incorrect");
    string_destroy_stk(&str3);
    
    TEST_PASS();
    return 0;
}


// Test 2: Append operations
int string_test_append(void)
{
    TEST_START("append operations");
    
    String* str = string_create();
    
    // Append cstr
    string_append_cstr(str, "hello");
    ASSERT(string_equals_cstr(str, "hello"), "append_cstr failed");
    
    string_append_cstr(str, " world");
    ASSERT(string_equals_cstr(str, "hello world"), "second append_cstr failed");
    
    // Append char
    string_append_char(str, '!');
    ASSERT(string_equals_cstr(str, "hello world!"), "append_char failed");
    ASSERT(string_len(str) == 12, "length after char append incorrect");
    
    // Append string
    String* str2 = string_from_cstr(" test");
    string_append_string(str, str2);
    ASSERT(string_equals_cstr(str, "hello world! test"), "append_string failed");
    
    string_destroy(str);
    string_destroy(str2);
    
    TEST_PASS();
    return 0;
}


// Test 3: Append with move
int string_test_append_move(void)
{
    TEST_START("append with move");
    
    String* str1 = string_from_cstr("start ");
    String* str2 = string_from_cstr("moved");
    
    string_append_string_move(str1, &str2);
    
    ASSERT(str2 == NULL, "source not nulled after move");
    ASSERT(string_equals_cstr(str1, "start moved"), "append_move content incorrect");
    
    string_destroy(str1);
    
    TEST_PASS();
    return 0;
}


// Test 4: Insert operations
int string_test_insert(void)
{
    TEST_START("insert operations");
    
    String* str = string_from_cstr("hello world");
    
    // Insert char
    string_insert_char(str, 5, ',');
    ASSERT(string_equals_cstr(str, "hello, world"), "insert_char failed");
    
    // Insert cstr at beginning
    string_insert_cstr(str, 0, "well ");
    ASSERT(string_equals_cstr(str, "well hello, world"), "insert_cstr at beginning failed");
    
    // Insert cstr in middle
    string_insert_cstr(str, 11, "beautiful ");
    ASSERT(string_equals_cstr(str, "well hello, beautiful world"), "insert_cstr in middle failed");
    
    // Insert string
    String* insert_str = string_from_cstr("my ");
    string_insert_string(str, 22, insert_str);
    ASSERT(string_equals_cstr(str, "well hello, beautiful my world"), "insert_string failed");
    
    string_destroy(str);
    string_destroy(insert_str);
    
    TEST_PASS();
    return 0;
}


// Test 5: Remove operations
int string_test_remove(void)
{
    TEST_START("remove operations");
    
    String* str = string_from_cstr("hello, world!");
    
    // Remove single char
    string_remove_char(str, 5);
    ASSERT(string_equals_cstr(str, "hello world!"), "remove_char failed");
    
    // Remove range
    string_remove_range(str, 5, 10); // Remove " world"
    ASSERT(string_equals_cstr(str, "hello!"), "remove_range failed");
    
    // Pop char
    char c = string_pop_char(str);
    ASSERT(c == '!', "pop_char returned wrong char");
    ASSERT(string_equals_cstr(str, "hello"), "string after pop incorrect");
    
    string_destroy(str);
    
    TEST_PASS();
    return 0;
}


// Test 6: Access and modification
int string_test_access(void)
{
    TEST_START("access and modification");
    
    String* str = string_from_cstr("hello");
    
    // Test char_at
    ASSERT(string_char_at(str, 0) == 'h', "char_at(0) failed");
    ASSERT(string_char_at(str, 4) == 'o', "char_at(4) failed");
    
    // Test set_char
    string_set_char(str, 0, 'H');
    ASSERT(string_char_at(str, 0) == 'H', "set_char failed");
    ASSERT(string_equals_cstr(str, "Hello"), "string after set_char incorrect");
    
    string_destroy(str);
    
    TEST_PASS();
    return 0;
}


// Test 7: Comparison operations
int string_test_compare(void)
{
    TEST_START("comparison operations");
    
    String* str1 = string_from_cstr("apple");
    String* str2 = string_from_cstr("banana");
    String* str3 = string_from_cstr("apple");
    
    // Test compare
    ASSERT(string_compare(str1, str2) < 0, "compare: apple should be < banana");
    ASSERT(string_compare(str2, str1) > 0, "compare: banana should be > apple");
    ASSERT(string_compare(str1, str3) == 0, "compare: apple should equal apple");
    
    // Test equals
    ASSERT(string_equals(str1, str3), "equals failed for identical strings");
    ASSERT(!string_equals(str1, str2), "equals should fail for different strings");
    
    // Test equals_cstr
    ASSERT(string_equals_cstr(str1, "apple"), "equals_cstr failed");
    ASSERT(!string_equals_cstr(str1, "orange"), "equals_cstr should fail");
    
    string_destroy(str1);
    string_destroy(str2);
    string_destroy(str3);
    
    TEST_PASS();
    return 0;
}


// Test 8: Search operations
int string_test_search(void)
{
    TEST_START("search operations");
    
    String* str = string_from_cstr("hello world, hello universe");
    
    // Find char
    u32 pos = string_find_char(str, 'w');
    ASSERT(pos == 6, "find_char 'w' failed");
    
    pos = string_find_char(str, 'z');
    ASSERT(pos == (u32)-1, "find_char should return -1 for not found");
    
    // Find cstr
    pos = string_find_cstr(str, "world");
    ASSERT(pos == 6, "find_cstr 'world' failed");
    
    pos = string_find_cstr(str, "hello");
    ASSERT(pos == 0, "find_cstr 'hello' should find first occurrence");
    
    pos = string_find_cstr(str, "missing");
    ASSERT(pos == (u32)-1, "find_cstr should return -1 for not found");
    
    string_destroy(str);
    
    TEST_PASS();
    return 0;
}


// Test 9: Substring operations
int string_test_substr(void)
{
    TEST_START("substring operations");
    
    String* str = string_from_cstr("hello world");
    
    // Extract "world"
    String* sub1 = string_substr(str, 6, 5);
    ASSERT(string_equals_cstr(sub1, "world"), "substr 'world' failed");
    
    // Extract "hello"
    String* sub2 = string_substr(str, 0, 5);
    ASSERT(string_equals_cstr(sub2, "hello"), "substr 'hello' failed");
    
    // Extract beyond end (should truncate)
    String* sub3 = string_substr(str, 6, 100);
    ASSERT(string_equals_cstr(sub3, "world"), "substr beyond end failed");
    
    string_destroy(str);
    string_destroy(sub1);
    string_destroy(sub2);
    string_destroy(sub3);
    
    TEST_PASS();
    return 0;
}


// Test 10: Copy and move semantics
int string_test_copy_move(void)
{
    TEST_START("copy and move semantics");
    
    // Test copy
    String* str1 = string_from_cstr("original");
    String str2;
    string_copy(&str2, str1);
    
    ASSERT(string_equals(str1, &str2), "copy: strings not equal");
    ASSERT(str1->buffer.data != str2.buffer.data, "copy: should have different data pointers");
    
    // Modify original, copy should be unchanged
    string_append_cstr(str1, " modified");
    ASSERT(!string_equals(str1, &str2), "copy: modification affected copy");
    ASSERT(string_equals_cstr(&str2, "original"), "copy was corrupted");
    
    string_destroy(str1);
    string_destroy_stk(&str2);
    
    // Test move
    String* str3 = string_from_cstr("move me");
    String str4;
    string_move(&str4, &str3);
    
    ASSERT(str3 == NULL, "move: source not nulled");
    ASSERT(string_equals_cstr(&str4, "move me"), "move: content incorrect");
    
    string_destroy_stk(&str4);
    
    // Test string_from_string (copy constructor)
    String* str5 = string_from_cstr("copy constructor");
    String* str6 = string_from_string(str5);
    
    ASSERT(string_equals(str5, str6), "from_string: strings not equal");
    ASSERT(str5->buffer.data != str6->buffer.data, "from_string: should have different data");
    
    string_destroy(str5);
    string_destroy(str6);
    
    TEST_PASS();
    return 0;
}


// Test 11: to_cstr operations
int string_test_to_cstr(void)
{
    TEST_START("to_cstr operations");
    
    String* str = string_from_cstr("test string");
    
    // Test to_cstr (returns copy with null terminator)
    const char* cstr = string_to_cstr(str);
    ASSERT(strcmp(cstr, "test string") == 0, "to_cstr failed");
    ASSERT(cstr[string_len(str)] == '\0', "to_cstr missing null terminator");
    free((void*)cstr); // Must free the copy!
    
    // Test to_cstr_ptr (returns pointer, no null terminator)
    const char* ptr = string_to_cstr_ptr(str);
    ASSERT(ptr == (const char*)str->buffer.data, "to_cstr_ptr not pointing to data");
    ASSERT(memcmp(ptr, "test string", string_len(str)) == 0, "to_cstr_ptr content wrong");
    // DO NOT free ptr - it points to internal buffer
    
    // Test empty string
    String* empty = string_create();
    const char* empty_cstr = string_to_cstr(empty);
    ASSERT(empty_cstr[0] == '\0', "empty to_cstr failed");
    free((void*)empty_cstr);
    
    const char* empty_ptr = string_to_cstr_ptr(empty);
    ASSERT(empty_ptr == NULL, "empty to_cstr_ptr should be NULL");
    
    string_destroy(str);
    string_destroy(empty);
    
    TEST_PASS();
    return 0;
}


// Test 12: Clear and reserve
int string_test_clear_reserve(void)
{
    TEST_START("clear and reserve");
    
    String* str = string_from_cstr("hello world");
    u32 old_capacity = str->buffer.capacity;
    
    // Clear should empty but keep capacity
    string_clear(str);
    ASSERT(string_len(str) == 0, "clear: length not 0");
    ASSERT(string_empty(str), "clear: should be empty");
    ASSERT(str->buffer.capacity == old_capacity, "clear: capacity changed");
    
    // Can still use after clear
    string_append_cstr(str, "new content");
    ASSERT(string_equals_cstr(str, "new content"), "clear: can't use after clear");
    
    // Reserve
    string_reserve(str, 100);
    ASSERT(str->buffer.capacity >= 100, "reserve failed");
    ASSERT(string_equals_cstr(str, "new content"), "reserve: content corrupted");
    
    string_destroy(str);
    
    TEST_PASS();
    return 0;
}


// Test 13: Edge cases
int string_test_edge_cases(void)
{
    TEST_START("edge cases");
    
    // Empty string operations
    String* empty = string_create();
    string_append_cstr(empty, "");
    ASSERT(string_len(empty) == 0, "appending empty cstr increased length");
    
    String* empty2 = string_create();
    string_append_string(empty, empty2);
    ASSERT(string_len(empty) == 0, "appending empty string increased length");
    
    // Insert at end (should behave like append)
    string_insert_cstr(empty, 0, "test");
    ASSERT(string_equals_cstr(empty, "test"), "insert at 0 failed");
    
    // Single char string
    String* single = string_from_cstr("x");
    ASSERT(string_len(single) == 1, "single char length wrong");
    char c = string_pop_char(single);
    ASSERT(c == 'x', "pop from single char wrong");
    ASSERT(string_empty(single), "should be empty after pop");
    
    string_destroy(empty);
    string_destroy(empty2);
    string_destroy(single);
    
    TEST_PASS();
    return 0;
}


// Test 14: Stress test - many operations
int string_test_stress(void)
{
    TEST_START("stress test");
    
    String* str = string_create();
    
    // Many appends
    for (int i = 0; i < 100; i++) {
        string_append_char(str, 'a' + (i % 26));
    }
    ASSERT(string_len(str) == 100, "stress: length after 100 appends wrong");
    
    // Many removes from end
    for (int i = 0; i < 50; i++) {
        string_pop_char(str);
    }
    ASSERT(string_len(str) == 50, "stress: length after 50 pops wrong");
    
    // Clear and rebuild
    string_clear(str);
    for (int i = 0; i < 1000; i++) {
        string_append_cstr(str, "x");
    }
    ASSERT(string_len(str) == 1000, "stress: length after 1000 appends wrong");
    
    string_destroy(str);
    
    TEST_PASS();
    return 0;
}


// Test runner
int run(void)
{
    printf("\n╔════════════════════════════════════╗\n");
    printf("║     STRING TEST SUITE              ║\n");
    printf("╚════════════════════════════════════╝\n");
    
    int failed = 0;
    
    failed += string_test_create_destroy();
    failed += string_test_append();
    failed += string_test_append_move();
    failed += string_test_insert();
    failed += string_test_remove();
    failed += string_test_access();
    failed += string_test_compare();
    failed += string_test_search();
    failed += string_test_substr();
    failed += string_test_copy_move();
    failed += string_test_to_cstr();
    failed += string_test_clear_reserve();
    failed += string_test_edge_cases();
    failed += string_test_stress();
    
    printf("\n╔════════════════════════════════════╗\n");
    if (failed == 0) {
        printf("║  ✓ ALL TESTS PASSED                ║\n");
    } else {
        printf("║  ✗ %d TEST(S) FAILED               ║\n", failed);
    }
    printf("╚════════════════════════════════════╝\n\n");
    
    return failed;
}
