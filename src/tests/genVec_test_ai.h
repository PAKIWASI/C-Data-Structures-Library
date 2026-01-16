#include "String.h"
#include "common.h"
#include "gen_vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test counter
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message)      \
    do {                                     \
        if (condition) {                     \
            tests_passed++;                  \
            printf("✓ PASS: %s\n", message); \
        } else {                             \
            tests_failed++;                  \
            printf("✗ FAIL: %s\n", message); \
        }                                    \
    } while (0)

#define TEST_START(name) printf("\n=== Testing %s ===\n", name)
#define TEST_END()       printf("\nTests passed: %d, Tests failed: %d\n", tests_passed, tests_failed)

// ============================================================================
// Helper functions for int vector tests
// ============================================================================

void int_print(const u8* elm)
{
    printf("%d", *(int*)elm);
}

// ============================================================================
// Helper functions for String* vector tests
// ============================================================================

void str_copy_ptr(u8* dest, const u8* src)
{
    String* s = *(String**)src;
    String* d = malloc(sizeof(String));
    memcpy(d, s, sizeof(String));
    u32 n          = s->buffer.size * s->buffer.data_size;
    d->buffer.data = malloc(n);
    memcpy(d->buffer.data, s->buffer.data, n);
    *(String**)dest = d;
}

void str_move_ptr(u8* dest, u8** src)
{
    *(String**)dest = *(String**)src;
    *src            = NULL;
}

void str_del_ptr(u8* elm)
{
    String* e = *(String**)elm;
    string_destroy(e);
}

void str_print_ptr(const u8* elm)
{
    string_print(*(const String**)elm);
}


// ============================================================================
// Test 1: Basic Initialization and Destruction
// ============================================================================

void test_init_destroy(void)
{
    TEST_START("init and destroy");

    // Test heap allocation
    genVec* vec = genVec_init(10, sizeof(int), NULL, NULL, NULL);
    TEST_ASSERT(vec != NULL, "heap init creates vector");
    TEST_ASSERT(vec->capacity == 10, "capacity set correctly");
    TEST_ASSERT(vec->size == 0, "size starts at 0");
    TEST_ASSERT(vec->data_size == sizeof(int), "data_size set correctly");
    genVec_destroy(vec);

    // Test stack allocation
    genVec v2;
    genVec_init_stk(5, sizeof(int), NULL, NULL, NULL, &v2);
    TEST_ASSERT(v2.capacity == 5, "stack init sets capacity");
    TEST_ASSERT(v2.size == 0, "stack init size starts at 0");
    genVec_destroy_stk(&v2);

    // Test init with value
    int     val = 42;
    genVec* v3  = genVec_init_val(5, (u8*)&val, sizeof(int), NULL, NULL, NULL);
    TEST_ASSERT(v3->size == 5, "init_val sets size");
    int retrieved;
    genVec_get(v3, 0, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 42, "init_val sets correct value");
    genVec_destroy(v3);
}

// ============================================================================
// Test 2: Push and Pop Operations
// ============================================================================

void test_push_pop(void)
{
    TEST_START("push and pop");

    genVec* vec = genVec_init(2, sizeof(int), NULL, NULL, NULL);

    // Test basic push
    int val1 = 10;
    int val2 = 20;
    int val3 = 30;
    genVec_push(vec, (u8*)&val1);
    TEST_ASSERT(vec->size == 1, "push increases size");

    genVec_push(vec, (u8*)&val2);
    TEST_ASSERT(vec->size == 2, "push increases size again");

    // Test auto-grow
    genVec_push(vec, (u8*)&val3);
    TEST_ASSERT(vec->size == 3, "push works after grow");
    TEST_ASSERT(vec->capacity > 2, "capacity increased");

    // Test pop
    int popped;
    genVec_pop(vec, (u8*)&popped);
    TEST_ASSERT(popped == 30, "pop returns correct value");
    TEST_ASSERT(vec->size == 2, "pop decreases size");

    genVec_pop(vec, (u8*)&popped);
    TEST_ASSERT(popped == 20, "pop returns correct value again");

    genVec_destroy(vec);
}

// ============================================================================
// Test 3: Get and Replace Operations
// ============================================================================

void test_get_replace(void)
{
    TEST_START("get and replace");

    genVec* vec = genVec_init(5, sizeof(int), NULL, NULL, NULL);

    // Push some values
    for (int i = 0; i < 5; i++) {
        int val = i * 10;
        genVec_push(vec, (u8*)&val);
    }

    // Test get
    int retrieved;
    genVec_get(vec, 2, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 20, "get returns correct value");

    // Test get_ptr
    const int* ptr = (const int*)genVec_get_ptr(vec, 3);
    TEST_ASSERT(*ptr == 30, "get_ptr returns correct pointer");

    // Test replace
    int new_val = 99;
    genVec_replace(vec, 2, (u8*)&new_val);
    genVec_get(vec, 2, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 99, "replace updates value");

    // Test front and back
    int* front = (int*)genVec_front(vec);
    int* back  = (int*)genVec_back(vec);
    TEST_ASSERT(*front == 0, "front returns first element");
    TEST_ASSERT(*back == 40, "back returns last element");

    genVec_destroy(vec);
}

// ============================================================================
// Test 4: Insert and Remove Operations
// ============================================================================

void test_insert_remove()
{
    TEST_START("insert and remove");

    genVec* vec = genVec_init(5, sizeof(int), NULL, NULL, NULL);

    // Push initial values: [10, 20, 30]
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) { genVec_push(vec, (u8*)&vals[i]); }

    // Test insert in middle
    int insert_val = 15;
    genVec_insert(vec, 1, (u8*)&insert_val);
    TEST_ASSERT(vec->size == 4, "insert increases size");

    int retrieved;
    genVec_get(vec, 1, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 15, "insert places value correctly");
    genVec_get(vec, 2, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 20, "insert shifts elements right");

    // Test insert at end
    int end_val = 40;
    genVec_insert(vec, vec->size, (u8*)&end_val);
    genVec_get(vec, vec->size - 1, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 40, "insert at end works");

    // Test remove from middle
    int removed;
    genVec_remove(vec, 1, (u8*)&removed);
    TEST_ASSERT(removed == 15, "remove returns correct value");
    TEST_ASSERT(vec->size == 4, "remove decreases size");
    genVec_get(vec, 1, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 20, "remove shifts elements left");

    genVec_destroy(vec);
}

// ============================================================================
// Test 5: Insert/Remove Multi Operations
// ============================================================================

void test_multi_operations()
{
    TEST_START("multi insert/remove");

    genVec* vec = genVec_init(10, sizeof(int), NULL, NULL, NULL);

    // Push initial values: [10, 20, 30]
    int vals[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) { genVec_push(vec, (u8*)&vals[i]); }

    // Test insert_multi
    int insert_vals[] = {15, 16, 17};
    genVec_insert_multi(vec, 1, (u8*)insert_vals, 3);
    TEST_ASSERT(vec->size == 6, "insert_multi increases size correctly");

    int retrieved;
    genVec_get(vec, 1, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 15, "insert_multi places first value");
    genVec_get(vec, 2, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 16, "insert_multi places second value");
    genVec_get(vec, 3, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 17, "insert_multi places third value");
    genVec_get(vec, 4, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 20, "insert_multi shifts existing elements");

    // Test remove_range
    genVec_remove_range(vec, 1, 3);
    TEST_ASSERT(vec->size == 3, "remove_range decreases size correctly");
    genVec_get(vec, 1, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 20, "remove_range removes correct elements");

    genVec_destroy(vec);
}

// ============================================================================
// Test 6: Clear and Reset Operations
// ============================================================================

void test_clear_reset(void)
{
    TEST_START("clear and reset");

    genVec* vec = genVec_init(10, sizeof(int), NULL, NULL, NULL);

    // Add some elements
    for (int i = 0; i < 5; i++) {
        int val = i;
        genVec_push(vec, (u8*)&val);
    }

    u32 old_capacity = vec->capacity;

    // Test clear
    genVec_clear(vec);
    TEST_ASSERT(vec->size == 0, "clear sets size to 0");
    TEST_ASSERT(vec->capacity == old_capacity, "clear preserves capacity");

    // Add elements again
    for (int i = 0; i < 3; i++) {
        int val = i * 10;
        genVec_push(vec, (u8*)&val);
    }

    // Test reset
    genVec_reset(vec);
    TEST_ASSERT(vec->size == 0, "reset sets size to 0");
    TEST_ASSERT(vec->capacity == 0, "reset sets capacity to 0");
    TEST_ASSERT(vec->data == NULL, "reset nulls data pointer");

    genVec_destroy(vec);
}

// ============================================================================
// Test 7: Reserve Operations
// ============================================================================

void test_reserve()
{
    TEST_START("reserve");

    genVec* vec = genVec_init(5, sizeof(int), NULL, NULL, NULL);

    // Test basic reserve
    genVec_reserve(vec, 20);
    TEST_ASSERT(vec->capacity == 20, "reserve increases capacity");
    TEST_ASSERT(vec->size == 0, "reserve doesn't change size");

    // Test reserve doesn't shrink
    genVec_reserve(vec, 10);
    TEST_ASSERT(vec->capacity == 20, "reserve doesn't shrink capacity");

    // Test reserve_val
    int val = 42;
    genVec_reserve_val(vec, 25, (u8*)&val);
    TEST_ASSERT(vec->capacity == 25, "reserve_val increases capacity");
    TEST_ASSERT(vec->size == 25, "reserve_val sets size");

    int retrieved;
    genVec_get(vec, 20, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 42, "reserve_val fills with correct value");

    genVec_destroy(vec);
}

// ============================================================================
// Test 8: Copy and Move Operations
// ============================================================================

void test_copy_move()
{
    TEST_START("copy and move");

    // Test copy
    genVec* vec1 = genVec_init(10, sizeof(int), NULL, NULL, NULL);
    for (int i = 0; i < 5; i++) {
        int val = i * 10;
        genVec_push(vec1, (u8*)&val);
    }

    genVec vec2;
    genVec_copy(&vec2, vec1);
    TEST_ASSERT(vec2.size == vec1->size, "copy preserves size");
    TEST_ASSERT(vec2.capacity == vec1->capacity, "copy preserves capacity");

    int retrieved1, retrieved2;
    genVec_get(vec1, 2, (u8*)&retrieved1);
    genVec_get(&vec2, 2, (u8*)&retrieved2);
    TEST_ASSERT(retrieved1 == retrieved2, "copy preserves data");

    // Modify original, check copy is independent
    int new_val = 99;
    genVec_replace(vec1, 2, (u8*)&new_val);
    genVec_get(&vec2, 2, (u8*)&retrieved2);
    TEST_ASSERT(retrieved2 == 20, "copy is independent of original");

    genVec_destroy_stk(&vec2);

    // Test move
    genVec vec3;
    genVec_move(&vec3, &vec1);
    TEST_ASSERT(vec1 == NULL, "move nulls source pointer");
    TEST_ASSERT(vec3.size == 5, "move transfers size");
    genVec_get(&vec3, 2, (u8*)&retrieved1);
    TEST_ASSERT(retrieved1 == 99, "move transfers data");

    genVec_destroy_stk(&vec3);
}

// ============================================================================
// Test 9: String Vector with Custom Functions
// ============================================================================

void test_string_vector()
{
    TEST_START("String* vector with custom functions");

    genVec* vec = genVec_init(5, sizeof(String*), str_copy_ptr, str_move_ptr, str_del_ptr);

    // Test push with copy
    String* s1 = string_from_cstr("hello");
    String* s2 = string_from_cstr("world");

    genVec_push(vec, (u8*)&s1);
    genVec_push(vec, (u8*)&s2);
    TEST_ASSERT(vec->size == 2, "push works with String*");

    // Verify we can still use original strings
    TEST_ASSERT(string_len(s1) == 5, "original string still valid after push");

    string_destroy(s1);
    string_destroy(s2);

    // Test that copied strings are independent
    String* retrieved = *(String**)genVec_get_ptr(vec, 0);
    TEST_ASSERT(string_equals_cstr(retrieved, "hello"), "retrieved correct string");

    // Test push_move
    String* s3 = string_from_cstr("moved");
    genVec_push_move(vec, (u8**)&s3);
    TEST_ASSERT(s3 == NULL, "push_move nulls source");
    TEST_ASSERT(vec->size == 3, "push_move increases size");

    // Test copy
    genVec vec2;
    genVec_copy(&vec2, vec);
    TEST_ASSERT(vec2.size == 3, "copy works with String*");

    String* str_from_copy = *(String**)genVec_get_ptr(&vec2, 0);
    TEST_ASSERT(string_equals_cstr(str_from_copy, "hello"), "copy preserves string data");

    genVec_destroy(vec);
    genVec_destroy_stk(&vec2);
}

// ============================================================================
// Test 10: Edge Cases and Stress Tests
// ============================================================================

void test_edge_cases()
{
    TEST_START("edge cases");

    // Test init with 0 capacity
    genVec* vec = genVec_init(0, sizeof(int), NULL, NULL, NULL);
    TEST_ASSERT(vec->capacity == 0, "init with 0 capacity works");
    TEST_ASSERT(vec->data == NULL, "init with 0 has NULL data");

    // Test push on 0 capacity vector
    int val = 42;
    genVec_push(vec, (u8*)&val);
    TEST_ASSERT(vec->size == 1, "push on 0 capacity grows vector");
    TEST_ASSERT(vec->capacity > 0, "capacity increased from 0");

    genVec_destroy(vec);

    // Test many push operations (stress test)
    genVec* vec2 = genVec_init(2, sizeof(int), NULL, NULL, NULL);
    for (int i = 0; i < 1000; i++) { genVec_push(vec2, (u8*)&i); }
    TEST_ASSERT(vec2->size == 1000, "stress test: 1000 pushes work");

    int retrieved;
    genVec_get(vec2, 999, (u8*)&retrieved);
    TEST_ASSERT(retrieved == 999, "stress test: data preserved");

    // Test many pops
    for (int i = 0; i < 500; i++) { genVec_pop(vec2, NULL); }
    TEST_ASSERT(vec2->size == 500, "stress test: 500 pops work");

    genVec_destroy(vec2);
}

// ============================================================================
// Test 11: Utility Functions
// ============================================================================

void test_utilities()
{
    TEST_START("utility functions");

    genVec* vec = genVec_init(10, sizeof(int), NULL, NULL, NULL);

    // Test empty
    TEST_ASSERT(genVec_empty(vec) == 1, "empty returns true for new vector");

    int val = 42;
    genVec_push(vec, (u8*)&val);
    TEST_ASSERT(genVec_empty(vec) == 0, "empty returns false after push");

    // Test size and capacity
    TEST_ASSERT(genVec_size(vec) == 1, "size returns correct value");
    TEST_ASSERT(genVec_capacity(vec) == 10, "capacity returns correct value");

    genVec_destroy(vec);
}

// ============================================================================
// Main Test Runner
// ============================================================================

int run(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║     COMPREHENSIVE genVec TEST SUITE                   ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");

    test_init_destroy();
    test_push_pop();
    test_get_replace();
    test_insert_remove();
    test_multi_operations();
    test_clear_reset();
    test_reserve();
    test_copy_move();
    test_string_vector();
    test_edge_cases();
    test_utilities();

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                   FINAL RESULTS                        ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  Total Tests Passed: %-5d                            ║\n", tests_passed);
    printf("║  Total Tests Failed: %-5d                            ║\n", tests_failed);
    printf("╚════════════════════════════════════════════════════════╝\n");

    if (tests_failed == 0) {
        printf("\n🎉 All tests passed! genVec is working correctly.\n\n");
        return 0;
    }

    printf("\n⚠️  Some tests failed. Please review the output above.\n\n");
    return 1;
}
