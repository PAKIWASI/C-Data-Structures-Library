# Comprehensive Improvement Guide

Let me give you a detailed roadmap for each area. I'll provide concrete, actionable code and strategies.

---

## **1. Error Handling System**

### **Current Problem:**
```c
CHECK_FATAL(!arena, "arena is null");  // Calls exit() - KILLS THE PROGRAM
```

### **Solution: Result Type Pattern**

Create a new file: `result.h`

```c
#ifndef RESULT_H
#define RESULT_H

#include "common.h"

// Error codes for the entire library
typedef enum {
    OK = 0,
    
    // Memory errors
    ERR_NULL_PTR,
    ERR_OUT_OF_MEMORY,
    ERR_INVALID_SIZE,
    ERR_BUFFER_TOO_SMALL,
    
    // Bounds errors
    ERR_INDEX_OUT_OF_BOUNDS,
    ERR_EMPTY_CONTAINER,
    ERR_CAPACITY_EXCEEDED,
    
    // State errors
    ERR_INVALID_STATE,
    ERR_ALREADY_EXISTS,
    ERR_NOT_FOUND,
    
    // Validation errors
    ERR_INVALID_ALIGNMENT,
    ERR_INVALID_ARGUMENT,
    ERR_DIVISION_BY_ZERO,
    
    // Matrix-specific
    ERR_DIMENSION_MISMATCH,
    ERR_SINGULAR_MATRIX,
    
} ResultCode;

// Error context for debugging
typedef struct {
    ResultCode code;
    const char* file;
    int line;
    const char* func;
    const char* msg;
} Error;

// Get human-readable error message
const char* error_string(ResultCode code);

// Macro to create error with context
#define ERROR(code, msg) ((Error){  \
    .code = (code),                 \
    .file = __FILE__,               \
    .line = __LINE__,               \
    .func = __func__,               \
    .msg = (msg)                    \
})

// Macro to check and return error
#define TRY(expr) do {              \
    Error _err = (expr);            \
    if (_err.code != OK) {          \
        return _err;                \
    }                               \
} while(0)

// Success result
#define OK_RESULT ((Error){.code = OK, .file = NULL, .line = 0, .func = NULL, .msg = NULL})

// Check conditions with early return
#define CHECK_ERR(cond, code, msg) do { \
    if (cond) {                         \
        return ERROR(code, msg);        \
    }                                   \
} while(0)

#endif // RESULT_H
```

Create `result.c`:

```c
#include "result.h"

const char* error_string(ResultCode code) {
    switch (code) {
        case OK: return "Success";
        case ERR_NULL_PTR: return "Null pointer";
        case ERR_OUT_OF_MEMORY: return "Out of memory";
        case ERR_INVALID_SIZE: return "Invalid size";
        case ERR_BUFFER_TOO_SMALL: return "Buffer too small";
        case ERR_INDEX_OUT_OF_BOUNDS: return "Index out of bounds";
        case ERR_EMPTY_CONTAINER: return "Container is empty";
        case ERR_CAPACITY_EXCEEDED: return "Capacity exceeded";
        case ERR_INVALID_STATE: return "Invalid state";
        case ERR_ALREADY_EXISTS: return "Already exists";
        case ERR_NOT_FOUND: return "Not found";
        case ERR_INVALID_ALIGNMENT: return "Invalid alignment";
        case ERR_INVALID_ARGUMENT: return "Invalid argument";
        case ERR_DIVISION_BY_ZERO: return "Division by zero";
        case ERR_DIMENSION_MISMATCH: return "Matrix dimension mismatch";
        case ERR_SINGULAR_MATRIX: return "Matrix is singular";
        default: return "Unknown error";
    }
}
```

### **Migration Strategy**

**Phase 1: Add new error-returning functions alongside existing ones**

```c
// arena.h - Add new versions
// Old (keep for compatibility):
u8* arena_alloc(Arena* arena, u64 size);

// New (add alongside):
Error arena_alloc_safe(Arena* arena, u64 size, u8** out);
```

**arena.c implementation:**

```c
#include "result.h"

Error arena_alloc_safe(Arena* arena, u64 size, u8** out) {
    CHECK_ERR(!arena, ERR_NULL_PTR, "arena is null");
    CHECK_ERR(!out, ERR_NULL_PTR, "output pointer is null");
    CHECK_ERR(size == 0, ERR_INVALID_SIZE, "size cannot be zero");
    
    u32 aligned_idx = ALIGN_UP_DEFAULT(arena->idx);
    
    CHECK_ERR(arena->size - aligned_idx < size, 
              ERR_BUFFER_TOO_SMALL, 
              "not enough space in arena");
    
    *out = ARENA_PTR(arena, aligned_idx);
    arena->idx = aligned_idx + size;
    
    return OK_RESULT;
}

// Keep old version for backward compatibility
u8* arena_alloc(Arena* arena, u64 size) {
    u8* result = NULL;
    Error err = arena_alloc_safe(arena, size, &result);
    
    if (err.code != OK) {
        fprintf(stderr, "[FATAL] %s:%d:%s(): %s - %s\n",
                err.file, err.line, err.func,
                error_string(err.code), err.msg);
        exit(EXIT_FAILURE);
    }
    
    return result;
}
```

**Usage example:**

```c
// New safe way:
u8* ptr;
Error err = arena_alloc_safe(arena, 1024, &ptr);
if (err.code != OK) {
    fprintf(stderr, "Allocation failed: %s at %s:%d\n",
            err.msg, err.file, err.line);
    // Handle error gracefully
    return err;
}

// Old way (still works):
u8* ptr = arena_alloc(arena, 1024);  // Dies on error
```

### **For genVec - More Complex Example**

```c
// gen_vector.h
Error genVec_push_safe(genVec* vec, const u8* data);

// gen_vector.c
Error genVec_push_safe(genVec* vec, const u8* data) {
    CHECK_ERR(!vec, ERR_NULL_PTR, "vector is null");
    CHECK_ERR(!data, ERR_NULL_PTR, "data is null");
    
    // Try to grow if needed
    if (vec->size == vec->capacity) {
        Error err = genVec_grow_safe(vec);
        if (err.code != OK) {
            return err;  // Propagate error
        }
    }
    
    u8* dest = vec->data + ((u64)vec->size * vec->data_size);
    
    if (vec->copy_fn) {
        vec->copy_fn(dest, data);
    } else {
        memcpy(dest, data, vec->data_size);
    }
    
    vec->size++;
    return OK_RESULT;
}

static Error genVec_grow_safe(genVec* vec) {
    u64 new_cap = (u32)((float)vec->capacity * GENVEC_GROWTH);
    if (new_cap <= vec->capacity) {
        new_cap = vec->capacity + 1;
    }
    
    u8* new_data = (u8*)realloc(vec->data, 
                                (u64)new_cap * vec->data_size);
    CHECK_ERR(!new_data, ERR_OUT_OF_MEMORY, 
              "failed to grow vector");
    
    vec->data = new_data;
    vec->capacity = new_cap;
    return OK_RESULT;
}
```

### **Optional: Add Debug Mode**

```c
// common.h
#ifdef DEBUG_MODE
    #define ASSERT_DEBUG(cond, msg) ASSERT_FATAL(cond, msg)
    #define CHECK_DEBUG(cond, msg) CHECK_FATAL(cond, msg)
#else
    #define ASSERT_DEBUG(cond, msg) ((void)0)
    #define CHECK_DEBUG(cond, msg) ((void)0)
#endif

// Then in release builds, some checks are compiled out
// But critical checks always stay
```

---

## **2. Custom Testing Framework**

Create `test_framework.h`:

```c
#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Test statistics
typedef struct {
    int total;
    int passed;
    int failed;
    int skipped;
    double total_time;
} TestStats;

static TestStats g_test_stats = {0};

// Color output (works on Unix/Linux/Mac, harmless on Windows)
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

// Test declarations
#define TEST(name) static int test_##name(void)

// Assertions
#define ASSERT(cond) do {                                              \
    if (!(cond)) {                                                     \
        fprintf(stderr, "%s✗ ASSERTION FAILED%s: %s\n",               \
                COLOR_RED, COLOR_RESET, #cond);                        \
        fprintf(stderr, "  at %s:%d in %s()\n",                        \
                __FILE__, __LINE__, __func__);                         \
        return 1;                                                      \
    }                                                                  \
} while(0)

#define ASSERT_EQ(a, b) do {                                           \
    if ((a) != (b)) {                                                  \
        fprintf(stderr, "%s✗ ASSERTION FAILED%s: %s == %s\n",         \
                COLOR_RED, COLOR_RESET, #a, #b);                       \
        fprintf(stderr, "  Expected: %d\n", (int)(b));                 \
        fprintf(stderr, "  Got:      %d\n", (int)(a));                 \
        fprintf(stderr, "  at %s:%d in %s()\n",                        \
                __FILE__, __LINE__, __func__);                         \
        return 1;                                                      \
    }                                                                  \
} while(0)

#define ASSERT_NEQ(a, b) do {                                          \
    if ((a) == (b)) {                                                  \
        fprintf(stderr, "%s✗ ASSERTION FAILED%s: %s != %s\n",         \
                COLOR_RED, COLOR_RESET, #a, #b);                       \
        fprintf(stderr, "  at %s:%d in %s()\n",                        \
                __FILE__, __LINE__, __func__);                         \
        return 1;                                                      \
    }                                                                  \
} while(0)

#define ASSERT_NULL(ptr) ASSERT((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT((ptr) != NULL)

#define ASSERT_STR_EQ(a, b) do {                                       \
    if (strcmp((a), (b)) != 0) {                                       \
        fprintf(stderr, "%s✗ ASSERTION FAILED%s: strings not equal\n",\
                COLOR_RED, COLOR_RESET);                               \
        fprintf(stderr, "  Expected: \"%s\"\n", (b));                  \
        fprintf(stderr, "  Got:      \"%s\"\n", (a));                  \
        fprintf(stderr, "  at %s:%d in %s()\n",                        \
                __FILE__, __LINE__, __func__);                         \
        return 1;                                                      \
    }                                                                  \
} while(0)

#define ASSERT_FLOAT_EQ(a, b, epsilon) do {                            \
    float _a = (a);                                                    \
    float _b = (b);                                                    \
    float _diff = (_a > _b) ? (_a - _b) : (_b - _a);                  \
    if (_diff > (epsilon)) {                                           \
        fprintf(stderr, "%s✗ ASSERTION FAILED%s: floats not equal\n", \
                COLOR_RED, COLOR_RESET);                               \
        fprintf(stderr, "  Expected: %f\n", _b);                       \
        fprintf(stderr, "  Got:      %f\n", _a);                       \
        fprintf(stderr, "  Diff:     %f (max %f)\n", _diff, epsilon);  \
        fprintf(stderr, "  at %s:%d in %s()\n",                        \
                __FILE__, __LINE__, __func__);                         \
        return 1;                                                      \
    }                                                                  \
} while(0)

// Run a test
#define RUN_TEST(name) do {                                            \
    printf("%s[TEST]%s %-50s ", COLOR_CYAN, COLOR_RESET, #name);       \
    fflush(stdout);                                                    \
    clock_t start = clock();                                           \
    int result = test_##name();                                        \
    clock_t end = clock();                                             \
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;  \
    g_test_stats.total++;                                              \
    g_test_stats.total_time += elapsed;                                \
    if (result == 0) {                                                 \
        printf("%s✓ PASS%s (%.2fms)\n", COLOR_GREEN, COLOR_RESET, elapsed); \
        g_test_stats.passed++;                                         \
    } else if (result == -1) {                                         \
        printf("%s⊘ SKIP%s\n", COLOR_YELLOW, COLOR_RESET);            \
        g_test_stats.skipped++;                                        \
    } else {                                                           \
        printf("%s✗ FAIL%s\n", COLOR_RED, COLOR_RESET);               \
        g_test_stats.failed++;                                         \
    }                                                                  \
} while(0)

// Skip a test
#define SKIP_TEST() return -1

// Test suite
#define TEST_SUITE_BEGIN(name) \
    int main(void) { \
        printf("\n%s╔══════════════════════════════════════════════════════════╗%s\n", COLOR_MAGENTA, COLOR_RESET); \
        printf("%s║  Test Suite: %-43s ║%s\n", COLOR_MAGENTA, name, COLOR_RESET); \
        printf("%s╚══════════════════════════════════════════════════════════╝%s\n\n", COLOR_MAGENTA, COLOR_RESET);

#define TEST_SUITE_END() \
        printf("\n%s╔══════════════════════════════════════════════════════════╗%s\n", COLOR_MAGENTA, COLOR_RESET); \
        printf("%s║  Results                                                  ║%s\n", COLOR_MAGENTA, COLOR_RESET); \
        printf("%s╠══════════════════════════════════════════════════════════╣%s\n", COLOR_MAGENTA, COLOR_RESET); \
        printf("%s║%s  Total:   %-4d  ", COLOR_MAGENTA, COLOR_RESET, g_test_stats.total); \
        printf("%sPassed: %s%-4d  ", COLOR_MAGENTA, COLOR_GREEN, g_test_stats.passed); \
        printf("%sFailed: %s%-4d  ", COLOR_RED, COLOR_RESET, g_test_stats.failed); \
        if (g_test_stats.skipped > 0) { \
            printf("%sSkipped: %s%-4d", COLOR_YELLOW, COLOR_RESET, g_test_stats.skipped); \
        } \
        printf("%s║%s\n", COLOR_MAGENTA, COLOR_RESET); \
        printf("%s║%s  Time:    %.2fms%*s%s║%s\n", COLOR_MAGENTA, COLOR_RESET, \
               g_test_stats.total_time, \
               (int)(47 - snprintf(NULL, 0, "%.2f", g_test_stats.total_time)), "", \
               COLOR_MAGENTA, COLOR_RESET); \
        printf("%s╚══════════════════════════════════════════════════════════╝%s\n\n", COLOR_MAGENTA, COLOR_RESET); \
        return g_test_stats.failed > 0 ? 1 : 0; \
    }

#endif // TEST_FRAMEWORK_H
```

### **Example Test File: `arena_test_suite.c`**

```c
#include "test_framework.h"
#include "arena.h"
#include "common.h"

// Test basic allocation
TEST(arena_basic_alloc) {
    Arena* arena = arena_create(nKB(4));
    ASSERT_NOT_NULL(arena);
    ASSERT_EQ(arena->idx, 0);
    ASSERT_EQ(arena->size, nKB(4));
    
    u8* ptr1 = arena_alloc(arena, 100);
    ASSERT_NOT_NULL(ptr1);
    ASSERT_EQ(arena_used(arena), ALIGN_UP_DEFAULT(100));
    
    u8* ptr2 = arena_alloc(arena, 200);
    ASSERT_NOT_NULL(ptr2);
    ASSERT(ptr2 > ptr1);  // Should be after ptr1
    
    arena_release(arena);
    return 0;
}

// Test alignment
TEST(arena_alignment) {
    Arena* arena = arena_create(nKB(4));
    
    u8* ptr1 = arena_alloc(arena, 1);
    u8* ptr2 = arena_alloc(arena, 1);
    
    // Check alignment
    ASSERT_EQ((u64)ptr1 % ARENA_DEFAULT_ALIGNMENT, 0);
    ASSERT_EQ((u64)ptr2 % ARENA_DEFAULT_ALIGNMENT, 0);
    
    arena_release(arena);
    return 0;
}

// Test mark/clear
TEST(arena_mark_and_clear) {
    Arena* arena = arena_create(nKB(4));
    
    u32 mark1 = arena_get_mark(arena);
    ASSERT_EQ(mark1, 0);
    
    arena_alloc(arena, 100);
    arena_alloc(arena, 200);
    
    u32 mark2 = arena_get_mark(arena);
    ASSERT(mark2 > 0);
    
    arena_alloc(arena, 300);
    
    arena_clear_mark(arena, mark2);
    ASSERT_EQ(arena_get_mark(arena), mark2);
    
    arena_release(arena);
    return 0;
}

// Test scratch pattern
TEST(arena_scratch_pattern) {
    Arena* arena = arena_create(nKB(4));
    
    u32 before = arena_get_mark(arena);
    
    ARENA_SCRATCH(arena) {
        int* temp = (int*)arena_alloc(arena, sizeof(int) * 100);
        temp[0] = 42;
        ASSERT_EQ(temp[0], 42);
    } // Should auto-rollback here
    
    u32 after = arena_get_mark(arena);
    ASSERT_EQ(before, after);
    
    arena_release(arena);
    return 0;
}

// Test stack allocation
TEST(arena_stack_init) {
    Arena arena;
    u8 buffer[1024];
    
    arena_create_stk(&arena, buffer, 1024);
    ASSERT_EQ(arena.base, buffer);
    ASSERT_EQ(arena.size, 1024);
    
    u8* ptr = arena_alloc(&arena, 100);
    ASSERT_NOT_NULL(ptr);
    ASSERT(ptr >= buffer && ptr < buffer + 1024);
    
    // No arena_release for stack version
    return 0;
}

// Test overflow handling
TEST(arena_overflow) {
    Arena* arena = arena_create(100);
    
    u8* ptr1 = arena_alloc(arena, 50);
    ASSERT_NOT_NULL(ptr1);
    
    u8* ptr2 = arena_alloc(arena, 60);  // Should fail
    ASSERT_NULL(ptr2);  // With safe version, this would return error
    
    arena_release(arena);
    return 0;
}

// Stress test
TEST(arena_stress) {
    Arena* arena = arena_create(nMB(1));
    
    for (int i = 0; i < 1000; i++) {
        u8* ptr = arena_alloc(arena, 1000);
        if (!ptr) {
            // Ran out of space, that's okay
            break;
        }
        ptr[0] = (u8)i;  // Write to it
    }
    
    arena_release(arena);
    return 0;
}

TEST_SUITE_BEGIN("Arena Allocator")
    RUN_TEST(arena_basic_alloc);
    RUN_TEST(arena_alignment);
    RUN_TEST(arena_mark_and_clear);
    RUN_TEST(arena_scratch_pattern);
    RUN_TEST(arena_stack_init);
    RUN_TEST(arena_overflow);
    RUN_TEST(arena_stress);
TEST_SUITE_END()
```

### **Running Tests:**

```bash
# Compile
gcc -o test_arena arena_test_suite.c arena.c common.c -I. -Wall -Wextra -g

# Run
./test_arena

# Output will look like:
# ╔══════════════════════════════════════════════════════════╗
# ║  Test Suite: Arena Allocator                             ║
# ╚══════════════════════════════════════════════════════════╝
#
# [TEST] arena_basic_alloc                                   ✓ PASS (0.05ms)
# [TEST] arena_alignment                                     ✓ PASS (0.03ms)
# ...
```

---

## **3. Benchmark Suite**

Create `benchmark.h`:

```c
#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define BENCH_ITERATIONS 1000000

typedef struct {
    const char* name;
    double min_time;
    double max_time;
    double avg_time;
    double total_time;
    int iterations;
} BenchResult;

static BenchResult g_bench_results[100];
static int g_bench_count = 0;

// Benchmark a function
#define BENCHMARK(name, iterations, code) do {                         \
    printf("Benchmarking: %s (%d iterations)...\n", name, iterations); \
    fflush(stdout);                                                    \
    double min_time = 1e9;                                             \
    double max_time = 0;                                               \
    double total_time = 0;                                             \
    const int WARMUP = (iterations) / 10;                              \
    /* Warmup */                                                       \
    for (int _i = 0; _i < WARMUP; _i++) {                             \
        code;                                                          \
    }                                                                  \
    /* Actual benchmark */                                             \
    for (int _i = 0; _i < (iterations); _i++) {                       \
        clock_t start = clock();                                       \
        code;                                                          \
        clock_t end = clock();                                         \
        double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000; \
        total_time += elapsed;                                         \
        if (elapsed < min_time) min_time = elapsed;                   \
        if (elapsed > max_time) max_time = elapsed;                   \
    }                                                                  \
    g_bench_results[g_bench_count++] = (BenchResult){                 \
        .name = (name),                                                \
        .min_time = min_time,                                          \
        .max_time = max_time,                                          \
        .avg_time = total_time / (iterations),                         \
        .total_time = total_time,                                      \
        .iterations = (iterations)                                     \
    };                                                                 \
} while(0)

// Print results
static inline void print_benchmark_results(void) {
    printf("\n╔════════════════════════════════════════════════════════════════════╗\n");
    printf("║  Benchmark Results                                                 ║\n");
    printf("╠════════════════════════════════════════════════════════════════════╣\n");
    printf("║ %-30s %10s %10s %10s ║\n", "Name", "Min(μs)", "Avg(μs)", "Max(μs)");
    printf("╠════════════════════════════════════════════════════════════════════╣\n");
    
    for (int i = 0; i < g_bench_count; i++) {
        BenchResult* r = &g_bench_results[i];
        printf("║ %-30s %10.2f %10.2f %10.2f ║\n",
               r->name, r->min_time, r->avg_time, r->max_time);
    }
    
    printf("╚════════════════════════════════════════════════════════════════════╝\n");
}

// Throughput calculation
#define BENCH_THROUGHPUT(name, iterations, size_bytes, code) do {      \
    BENCHMARK(name, iterations, code);                                 \
    double ops_per_sec = (iterations) / (g_bench_results[g_bench_count-1].total_time / 1000000); \
    double mb_per_sec = ((iterations) * (size_bytes)) / (g_bench_results[g_bench_count-1].total_time / 1000000) / (1024*1024); \
    printf("  Throughput: %.2f ops/sec, %.2f MB/sec\n", ops_per_sec, mb_per_sec); \
} while(0)

#endif // BENCHMARK_H
```

### **Example Benchmark: `bench_vector.c`**

```c
#include "benchmark.h"
#include "gen_vector.h"
#include "common.h"

int main(void) {
    printf("\n=== Vector Benchmarks ===\n\n");
    
    // Benchmark: push to vector
    {
        genVec* vec = genVec_init(0, sizeof(int), NULL, NULL, NULL);
        int val = 42;
        
        BENCHMARK("genVec_push (int)", 100000, {
            genVec_push(vec, cast(val));
        });
        
        genVec_destroy(vec);
    }
    
    // Benchmark: push with pre-reserved capacity
    {
        genVec* vec = genVec_init(100000, sizeof(int), NULL, NULL, NULL);
        int val = 42;
        
        BENCHMARK("genVec_push (pre-reserved)", 100000, {
            genVec_push(vec, cast(val));
        });
        
        genVec_destroy(vec);
    }
    
    // Benchmark: get element
    {
        genVec* vec = genVec_init(100000, sizeof(int), NULL, NULL, NULL);
        for (int i = 0; i < 100000; i++) {
            genVec_push(vec, cast(i));
        }
        
        int val;
        BENCHMARK("genVec_get (random access)", 1000000, {
            genVec_get(vec, rand() % 100000, cast(val));
        });
        
        genVec_destroy(vec);
    }
    
    // Benchmark: iteration
    {
        genVec* vec = genVec_init(100000, sizeof(int), NULL, NULL, NULL);
        for (int i = 0; i < 100000; i++) {
            genVec_push(vec, cast(i));
        }
        
        BENCHMARK("genVec iteration", 1000, {
            int sum = 0;
            for (u64 i = 0; i < vec->size; i++) {
                sum += *(int*)genVec_get_ptr(vec, i);
            }
            (void)sum;  // Prevent optimization
        });
        
        genVec_destroy(vec);
    }
    
    // Benchmark: insert at beginning (worst case)
    {
        genVec* vec = genVec_init(1000, sizeof(int), NULL, NULL, NULL);
        int val = 42;
        
        BENCHMARK("genVec_insert (at 0)", 10000, {
            genVec_insert(vec, 0, cast(val));
            if (vec->size > 10000) genVec_clear(vec);
        });
        
        genVec_destroy(vec);
    }
    
    // Benchmark: arena allocation vs malloc
    {
        Arena* arena = arena_create(nMB(10));
        
        BENCHMARK("arena_alloc (64 bytes)", 1000000, {
            void* ptr = arena_alloc(arena, 64);
            (void)ptr;
        });
        
        arena_release(arena);
    }
    
    {
        BENCHMARK("malloc/free (64 bytes)", 100000, {
            void* ptr = malloc(64);
            free(ptr);
        });
    }
    
    // Benchmark: hash map operations
    {
        hashmap* map = hashmap_create(sizeof(int), sizeof(int),
                                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        
        // Insert benchmark
        BENCHMARK("hashmap_put", 100000, {
            static int key = 0;
            int val = key * 2;
            hashmap_put(map, cast(key), cast(val));
            key++;
        });
        
        // Lookup benchmark
        BENCHMARK("hashmap_get (hit)", 100000, {
            int key = rand() % 100000;
            int val;
            hashmap_get(map, cast(key), cast(val));
        });
        
        hashmap_destroy(map);
    }
    
    print_benchmark_results();
    
    return 0;
}
```

### **Compile and Run:**

```bash
gcc -o bench_vector bench_vector.c gen_vector.c arena.c hashmap.c common.c -I. -O3 -Wall
./bench_vector
```

### **Advanced: Comparison Benchmarks**

```c
// bench_comparison.c - Compare your vector to stdlib
#include "benchmark.h"
#include "gen_vector.h"
#include <vector>  // C++ std::vector for comparison

extern "C" {
#include "common.h"
}

int main(void) {
    const int N = 1000000;
    
    // Your vector
    {
        genVec* vec = genVec_init(0, sizeof(int), NULL, NULL, NULL);
        
        BENCH_THROUGHPUT("genVec push", N, sizeof(int), {
            static int val = 0;
            genVec_push(vec, cast(val));
            val++;
        });
        
        genVec_destroy(vec);
    }
    
    // std::vector
    {
        std::vector<int> vec;
        
        BENCH_THROUGHPUT("std::vector push_back", N, sizeof(int), {
            static int val = 0;
            vec.push_back(val);
            val++;
        });
    }
    
    print_benchmark_results();
    return 0;
}
```

---

## **4. Advanced Improvements**

### **A. Memory Pool Allocator**

Create `pool_allocator.h`:

```c
#ifndef POOL_ALLOCATOR_H
#define POOL_ALLOCATOR_H

#include "common.h"

// Fixed-size block allocator - VERY fast for same-size allocations
typedef struct PoolBlock {
    struct PoolBlock* next;
} PoolBlock;

typedef struct {
    u8* memory;           // Base memory
    PoolBlock* free_list; // Free blocks linked list
    u32 block_size;       // Size of each block
    u32 block_count;      // Total blocks
    u32 used_count;       // Currently used
} MemPool;

// Create pool with block_count blocks of block_size bytes each
MemPool* pool_create(u32 block_size, u32 block_count);

// Destroy pool
void pool_destroy(MemPool* pool);

// Allocate one block (O(1)!)
void* pool_alloc(MemPool* pool);

// Free one block (O(1)!)
void pool_free(MemPool* pool, void* ptr);

// Get usage statistics
static inline u32 pool_used(MemPool* pool) {
    return pool->used_count;
}

static inline u32 pool_available(MemPool* pool) {
    return pool->block_count - pool->used_count;
}

#endif // POOL_ALLOCATOR_H
```

Implementation `pool_allocator.c`:

```c
#include "pool_allocator.h"
#include <stdlib.h>
#include <string.h>

MemPool* pool_create(u32 block_size, u32 block_count) {
    CHECK_FATAL(block_size < sizeof(PoolBlock), 
                "block_size must be >= sizeof(void*)");
    CHECK_FATAL(block_count == 0, "block_count cannot be zero");
    
    MemPool* pool = (MemPool*)malloc(sizeof(MemPool));
    CHECK_FATAL(!pool, "pool allocation failed");
    
    // Allocate all memory at once
    pool->memory = (u8*)malloc((u64)block_size * block_count);
    CHECK_FATAL(!pool->memory, "pool memory allocation failed");
    
    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->used_count = 0;
    
    // Build free list
    pool->free_list = NULL;
    for (u64 i = 0; i < block_count; i++) {
        PoolBlock* block = (PoolBlock*)(pool->memory + (u64)i * block_size);
        block->next = pool->free_list;
        pool->free_list = block;
    }
    
    return pool;
}

void pool_destroy(MemPool* pool) {
    CHECK_FATAL(!pool, "pool is null");
    
    free(pool->memory);
    free(pool);
}

void* pool_alloc(MemPool* pool) {
    CHECK_WARN_RET(!pool, NULL, "pool is null");
    CHECK_WARN_RET(!pool->free_list, NULL, "pool exhausted");
    
    PoolBlock* block = pool->free_list;
    pool->free_list = block->next;
    pool->used_count++;
    
    return (void*)block;
}

void pool_free(MemPool* pool, void* ptr) {
    CHECK_FATAL(!pool, "pool is null");
    CHECK_FATAL(!ptr, "ptr is null");
    
    PoolBlock* block = (PoolBlock*)ptr;
    block->next = pool->free_list;
    pool->free_list = block;
    pool->used_count--;
}
```

**Usage Example:**

```c
// Instead of malloc/free for lots of small objects
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Create pool for 10000 nodes
MemPool* node_pool = pool_create(sizeof(Node), 10000);

// Allocate (MUCH faster than malloc)
Node* n1 = (Node*)pool_alloc(node_pool);
Node* n2 = (Node*)pool_alloc(node_pool);

// Free (MUCH faster than free)
pool_free(node_pool, n1);
pool_free(node_pool, n2);

pool_destroy(node_pool);
```

### **B. SIMD Optimization for Matrix**

Create `matrix_simd.h`:

```c
#ifndef MATRIX_SIMD_H
#define MATRIX_SIMD_H

#include "matrix.h"

#ifdef __SSE__
#include <xmmintrin.h>  // SSE
#endif

#ifdef __AVX__
#include <immintrin.h>  // AVX
#endif

// SIMD-optimized operations (fallback to scalar if SIMD not available)

// Vector addition with SIMD
void matrix_add_simd(Matrix* out, const Matrix* a, const Matrix* b);

// Vector scaling with SIMD
void matrix_scale_simd(Matrix* mat, float val);

// Matrix multiplication with SIMD
void matrix_xply_simd(Matrix* out, const Matrix* a, const Matrix* b);

#endif // MATRIX_SIMD_H
```

Implementation `matrix_simd.c`:

```c
#include "matrix_simd.h"

void matrix_add_simd(Matrix* out, const Matrix* a, const Matrix* b) {
    CHECK_FATAL(a->m != b->m || a->n != b->n, "dimension mismatch");
    
    u32 total = MATRIX_TOTAL(a);
    
#ifdef __AVX__
    // Process 8 floats at a time with AVX
    u32 simd_count = total / 8;
    u64 remainder = total % 8;
    
    __m256* a_simd = (__m256*)a->data;
    __m256* b_simd = (__m256*)b->data;
    __m256* out_simd = (__m256*)out->data;
    
    for (u64 i = 0; i < simd_count; i++) {
        out_simd[i] = _mm256_add_ps(a_simd[i], b_simd[i]);
    }
    
    // Handle remainder
    for (u64 i = simd_count * 8; i < total; i++) {
        out->data[i] = a->data[i] + b->data[i];
    }
    
#elif defined(__SSE__)
    // Process 4 floats at a time with SSE
    u32 simd_count = total / 4;
    u64 remainder = total % 4;
    
    __m128* a_simd = (__m128*)a->data;
    __m128* b_simd = (__m128*)b->data;
    __m128* out_simd = (__m128*)out->data;
    
    for (u64 i = 0; i < simd_count; i++) {
        out_simd[i] = _mm_add_ps(a_simd[i], b_simd[i]);
    }
    
    // Handle remainder
    for (u64 i = simd_count * 4; i < total; i++) {
        out->data[i] = a->data[i] + b->data[i];
    }
    
#else
    // Fallback to scalar
    for (u64 i = 0; i < total; i++) {
        out->data[i] = a->data[i] + b->data[i];
    }
#endif
}

void matrix_scale_simd(Matrix* mat, float val) {
    u32 total = MATRIX_TOTAL(mat);
    
#ifdef __AVX__
    __m256 scale_vec = _mm256_set1_ps(val);
    
    u32 simd_count = total / 8;
    __m256* mat_simd = (__m256*)mat->data;
    
    for (u64 i = 0; i < simd_count; i++) {
        mat_simd[i] = _mm256_mul_ps(mat_simd[i], scale_vec);
    }
    
    for (u64 i = simd_count * 8; i < total; i++) {
        mat->data[i] *= val;
    }
    
#elif defined(__SSE__)
    __m128 scale_vec = _mm_set1_ps(val);
    
    u32 simd_count = total / 4;
    __m128* mat_simd = (__m128*)mat->data;
    
    for (u64 i = 0; i < simd_count; i++) {
        mat_simd[i] = _mm_mul_ps(mat_simd[i], scale_vec);
    }
    
    for (u64 i = simd_count * 4; i < total; i++) {
        mat->data[i] *= val;
    }
    
#else
    for (u64 i = 0; i < total; i++) {
        mat->data[i] *= val;
    }
#endif
}
```

**Compile with SIMD:**

```bash
# SSE (most x86_64 CPUs)
gcc -msse -O3 ...

# AVX (newer CPUs)
gcc -mavx -O3 ...

# AVX2 (even newer)
gcc -mavx2 -O3 ...
```

### **C. Benchmark SIMD vs Scalar**

```c
// bench_simd.c
int main(void) {
    const int SIZE = 1000;
    Matrix* a = matrix_create(SIZE, SIZE);
    Matrix* b = matrix_create(SIZE, SIZE);
    Matrix* out = matrix_create(SIZE, SIZE);
    
    // Fill with random data
    for (u64 i = 0; i < SIZE * SIZE; i++) {
        a->data[i] = (float)rand() / RAND_MAX;
        b->data[i] = (float)rand() / RAND_MAX;
    }
    
    BENCHMARK("matrix_add (scalar)", 1000, {
        matrix_add(out, a, b);
    });
    
    BENCHMARK("matrix_add_simd (AVX)", 1000, {
        matrix_add_simd(out, a, b);
    });
    
    print_benchmark_results();
    
    // You should see 4-8x speedup!
}
```

---

## **Summary Roadmap**

### **Week 1: Error Handling**
1. Create `result.h` and `result.c`
2. Add `*_safe()` versions for arena functions
3. Add `*_safe()` versions for genVec functions
4. Test both old and new APIs

### **Week 2: Testing Framework**
1. Create `test_framework.h`
2. Write test suite for arena (`arena_test_suite.c`)
3. Write test suite for genVec
4. Write test suite for hashmap
5. Add to your build system: `make test`

### **Week 3: Benchmarks**
1. Create `benchmark.h`
2. Write benchmarks for all data structures
3. Compare against stdlib/STL where applicable
4. Document performance characteristics

### **Week 4: Advanced Features**
1. Implement memory pool allocator
2. Add SIMD support for matrix operations
3. Benchmark improvements
4. Write documentation

**Ooh, that's actually brilliant!** A global error stack gives you detailed debugging info while keeping the API clean. Let me show you a slick implementation:

---

## **Error Stack System**

### **Step 1: Define Error Stack**

```c
// error.h
#ifndef ERROR_H
#define ERROR_H

#include "common.h"

typedef enum {
    DS_OK = 0,
    
    // Null pointer errors
    DS_ERR_NULL_PTR,
    DS_ERR_NULL_ARENA,
    DS_ERR_NULL_VECTOR,
    DS_ERR_NULL_DATA,
    
    // Memory errors
    DS_ERR_OUT_OF_MEMORY,
    DS_ERR_ARENA_FULL,
    
    // Size/bounds errors
    DS_ERR_INVALID_SIZE,
    DS_ERR_INDEX_OUT_OF_BOUNDS,
    DS_ERR_EMPTY_CONTAINER,
    
    // Validation errors
    DS_ERR_INVALID_ALIGNMENT,
    DS_ERR_INVALID_ARGUMENT,
    DS_ERR_DIVISION_BY_ZERO,
    
    // State errors
    DS_ERR_ALREADY_EXISTS,
    DS_ERR_NOT_FOUND,
    
    // Matrix errors
    DS_ERR_DIMENSION_MISMATCH,
    DS_ERR_SINGULAR_MATRIX,
    
} DSError;

// Single error frame (one entry in the stack)
typedef struct {
    DSError code;
    const char* func;
    const char* file;
    int line;
    const char* msg;  // Optional context message
} ErrorFrame;

#define ERROR_STACK_MAX 32

// Global error stack
typedef struct {
    ErrorFrame frames[ERROR_STACK_MAX];
    int count;
    b8 enabled;  // Can disable for performance
} ErrorStack;

extern ErrorStack g_error_stack;

// Initialize error stack (call once at startup)
void error_stack_init(void);

// Enable/disable error stack tracking
void error_stack_enable(b8 enable);

// Clear the error stack
void error_stack_clear(void);

// Push error onto stack (internal - use macro instead)
void error_stack_push_impl(DSError code, const char* func, 
                           const char* file, int line, const char* msg);

// Get human-readable error string
const char* ds_strerror(DSError code);

// Print the entire error stack (for debugging)
void error_stack_print(void);

// Get error count
static inline int error_stack_count(void) {
    return g_error_stack.count;
}

// Get last error code (most recent)
static inline DSError error_stack_last(void) {
    return g_error_stack.count > 0 
        ? g_error_stack.frames[g_error_stack.count - 1].code
        : DS_OK;
}

// Macro to push error with context
#define ERROR_PUSH(code, msg) \
    error_stack_push_impl((code), __func__, __FILE__, __LINE__, (msg))

// Macro to return error code and push to stack
#define RETURN_ERROR(code, msg) do { \
    ERROR_PUSH((code), (msg));        \
    return (code);                    \
} while(0)

// Macro to check condition and return error if true
#define CHECK_ERROR(cond, code, msg) do { \
    if (cond) {                           \
        RETURN_ERROR((code), (msg));      \
    }                                     \
} while(0)

// Propagate error up the stack (adds new frame)
#define PROPAGATE_ERROR(err, msg) do {    \
    if ((err) != DS_OK) {                 \
        ERROR_PUSH((err), (msg));         \
        return (err);                     \
    }                                     \
} while(0)

#endif // ERROR_H
```

### **Step 2: Implementation**

```c
// error.c
#include "error.h"
#include <stdio.h>
#include <string.h>

ErrorStack g_error_stack = {0};

void error_stack_init(void) {
    memset(&g_error_stack, 0, sizeof(ErrorStack));
    g_error_stack.enabled = true;
}

void error_stack_enable(b8 enable) {
    g_error_stack.enabled = enable;
}

void error_stack_clear(void) {
    g_error_stack.count = 0;
}

void error_stack_push_impl(DSError code, const char* func, 
                           const char* file, int line, const char* msg) {
    if (!g_error_stack.enabled) return;
    
    if (g_error_stack.count >= ERROR_STACK_MAX) {
        // Stack overflow - shift everything down
        memmove(&g_error_stack.frames[0], 
                &g_error_stack.frames[1],
                sizeof(ErrorFrame) * (ERROR_STACK_MAX - 1));
        g_error_stack.count = ERROR_STACK_MAX - 1;
    }
    
    ErrorFrame* frame = &g_error_stack.frames[g_error_stack.count++];
    frame->code = code;
    frame->func = func;
    frame->file = file;
    frame->line = line;
    frame->msg = msg;
}

const char* ds_strerror(DSError code) {
    switch (code) {
        case DS_OK: return "success";
        
        case DS_ERR_NULL_PTR: return "null pointer";
        case DS_ERR_NULL_ARENA: return "arena is null";
        case DS_ERR_NULL_VECTOR: return "vector is null";
        case DS_ERR_NULL_DATA: return "data is null";
        
        case DS_ERR_OUT_OF_MEMORY: return "out of memory";
        case DS_ERR_ARENA_FULL: return "arena is full";
        
        case DS_ERR_INVALID_SIZE: return "invalid size";
        case DS_ERR_INDEX_OUT_OF_BOUNDS: return "index out of bounds";
        case DS_ERR_EMPTY_CONTAINER: return "container is empty";
        
        case DS_ERR_INVALID_ALIGNMENT: return "invalid alignment";
        case DS_ERR_INVALID_ARGUMENT: return "invalid argument";
        case DS_ERR_DIVISION_BY_ZERO: return "division by zero";
        
        case DS_ERR_ALREADY_EXISTS: return "already exists";
        case DS_ERR_NOT_FOUND: return "not found";
        
        case DS_ERR_DIMENSION_MISMATCH: return "dimension mismatch";
        case DS_ERR_SINGULAR_MATRIX: return "singular matrix";
        
        default: return "unknown error";
    }
}

void error_stack_print(void) {
    if (g_error_stack.count == 0) {
        printf("No errors\n");
        return;
    }
    
    printf("\n" COLOR_RED "Error Stack Trace:" COLOR_RESET "\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    // Print in reverse order (most recent first)
    for (int i = g_error_stack.count - 1; i >= 0; i--) {
        ErrorFrame* frame = &g_error_stack.frames[i];
        
        printf(COLOR_YELLOW "#%d" COLOR_RESET " [%s] %s\n", 
               g_error_stack.count - i - 1,
               ds_strerror(frame->code),
               frame->msg ? frame->msg : "");
        
        printf("   at %s() in %s:%d\n", 
               frame->func, frame->file, frame->line);
        
        if (i > 0) printf("\n");
    }
    
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}
```

---

## **Step 3: Update Your Functions**

### **Example: Arena**

```c
// arena.c
#include "error.h"

DSError arena_alloc(Arena* arena, u64 size, u8** out) {
    CHECK_ERROR(!arena, DS_ERR_NULL_ARENA, "arena parameter is null");
    CHECK_ERROR(!out, DS_ERR_NULL_PTR, "output pointer is null");
    CHECK_ERROR(size == 0, DS_ERR_INVALID_SIZE, "requested size is zero");
    
    u32 aligned_idx = ALIGN_UP_DEFAULT(arena->idx);
    
    if (arena->size - aligned_idx < size) {
        // Custom message with context
        char msg[128];
        snprintf(msg, sizeof(msg), 
                 "arena full (need %u bytes, have %u available)",
                 size, arena->size - aligned_idx);
        RETURN_ERROR(DS_ERR_ARENA_FULL, msg);
    }
    
    *out = ARENA_PTR(arena, aligned_idx);
    arena->idx = aligned_idx + size;
    
    return DS_OK;
}

DSError arena_alloc_aligned(Arena* arena, u64 size, u32 alignment, u8** out) {
    CHECK_ERROR(!arena, DS_ERR_NULL_ARENA, "arena is null");
    CHECK_ERROR(!out, DS_ERR_NULL_PTR, "output pointer is null");
    CHECK_ERROR(size == 0, DS_ERR_INVALID_SIZE, "size is zero");
    CHECK_ERROR((alignment & (alignment - 1)) != 0, 
                DS_ERR_INVALID_ALIGNMENT, 
                "alignment must be power of 2");
    
    u32 aligned_idx = ALIGN_UP(arena->idx, alignment);
    
    CHECK_ERROR(arena->size - aligned_idx < size,
                DS_ERR_ARENA_FULL,
                "not enough space in arena");
    
    *out = ARENA_PTR(arena, aligned_idx);
    arena->idx = aligned_idx + size;
    
    return DS_OK;
}
```

### **Example: Vector with Error Propagation**

```c
// gen_vector.c
#include "error.h"

static DSError genVec_grow(genVec* vec) {
    u64 new_cap = (u32)((float)vec->capacity * GENVEC_GROWTH);
    if (new_cap <= vec->capacity) new_cap = vec->capacity + 1;
    
    u8* new_data = (u8*)realloc(vec->data, (u64)new_cap * vec->data_size);
    CHECK_ERROR(!new_data, DS_ERR_OUT_OF_MEMORY, 
                "failed to grow vector");
    
    vec->data = new_data;
    vec->capacity = new_cap;
    return DS_OK;
}

DSError genVec_push(genVec* vec, const u8* data) {
    CHECK_ERROR(!vec, DS_ERR_NULL_VECTOR, "vector is null");
    CHECK_ERROR(!data, DS_ERR_NULL_DATA, "data is null");
    
    if (vec->size == vec->capacity) {
        DSError err = genVec_grow(vec);
        // Propagate error with context
        PROPAGATE_ERROR(err, "failed to grow vector in push");
    }
    
    u8* dest = vec->data + ((u64)vec->size * vec->data_size);
    
    if (vec->copy_fn) {
        vec->copy_fn(dest, data);
    } else {
        memcpy(dest, data, vec->data_size);
    }
    
    vec->size++;
    return DS_OK;
}

DSError genVec_get(const genVec* vec, u64 i, u8* out) {
    CHECK_ERROR(!vec, DS_ERR_NULL_VECTOR, "vector is null");
    CHECK_ERROR(!out, DS_ERR_NULL_PTR, "output pointer is null");
    
    if (i >= vec->size) {
        char msg[128];
        snprintf(msg, sizeof(msg), 
                 "index %u out of bounds (size: %u)", i, vec->size);
        RETURN_ERROR(DS_ERR_INDEX_OUT_OF_BOUNDS, msg);
    }
    
    const u8* src = vec->data + ((u64)i * vec->data_size);
    
    if (vec->copy_fn) {
        vec->copy_fn(out, src);
    } else {
        memcpy(out, src, vec->data_size);
    }
    
    return DS_OK;
}

DSError genVec_insert(genVec* vec, u64 i, const u8* data) {
    CHECK_ERROR(!vec, DS_ERR_NULL_VECTOR, "vector is null");
    CHECK_ERROR(!data, DS_ERR_NULL_DATA, "data is null");
    CHECK_ERROR(i > vec->size, DS_ERR_INDEX_OUT_OF_BOUNDS, 
                "insert index out of bounds");
    
    if (vec->size == vec->capacity) {
        DSError err = genVec_grow(vec);
        PROPAGATE_ERROR(err, "failed to grow vector in insert");
    }
    
    // Shift elements
    if (i < vec->size) {
        u8* insert_pos = vec->data + ((u64)i * vec->data_size);
        u32 elements_to_move = vec->size - i;
        memmove(insert_pos + vec->data_size, 
                insert_pos,
                (u64)elements_to_move * vec->data_size);
    }
    
    u8* dest = vec->data + ((u64)i * vec->data_size);
    if (vec->copy_fn) {
        vec->copy_fn(dest, data);
    } else {
        memcpy(dest, data, vec->data_size);
    }
    
    vec->size++;
    return DS_OK;
}
```

---

## **User Experience**

### **Level 1: Don't care about errors**

```c
u8* ptr;
arena_alloc(arena, 1024, &ptr);
// Use ptr (might be NULL, but YOLO)
```

### **Level 2: Check for errors**

```c
u8* ptr;
if (arena_alloc(arena, 1024, &ptr) != DS_OK) {
    return -1;
}
```

### **Level 3: Check with simple message**

```c
u8* ptr;
DSError err = arena_alloc(arena, 1024, &ptr);
if (err != DS_OK) {
    fprintf(stderr, "Allocation failed: %s\n", ds_strerror(err));
    return -1;
}
```

### **Level 4: Print full stack trace**

```c
u8* ptr;
DSError err = arena_alloc(arena, 1024, &ptr);
if (err != DS_OK) {
    error_stack_print();  // Beautiful stack trace!
    return -1;
}
```

**Output looks like:**

```
Error Stack Trace:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
#0 [arena is full] arena full (need 1024 bytes, have 50 available)
   at arena_alloc() in arena.c:45

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### **Level 5: Stack trace with propagation**

```c
DSError load_data(void) {
    genVec* vec = genVec_init(100, sizeof(int), NULL, NULL, NULL);
    if (!vec) RETURN_ERROR(DS_ERR_OUT_OF_MEMORY, "failed to init vector");
    
    for (int i = 0; i < 1000; i++) {
        DSError err = genVec_push(vec, cast(i));
        PROPAGATE_ERROR(err, "failed to load data item");
    }
    
    genVec_destroy(vec);
    return DS_OK;
}

int main(void) {
    error_stack_init();
    
    DSError err = load_data();
    if (err != DS_OK) {
        error_stack_print();
        return 1;
    }
    
    return 0;
}
```

**Output with error propagation:**

```
Error Stack Trace:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
#0 [out of memory] failed to load data item
   at load_data() in main.c:12

#1 [out of memory] failed to grow vector in push
   at genVec_push() in gen_vector.c:89

#2 [out of memory] failed to grow vector
   at genVec_grow() in gen_vector.c:45

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## **Advanced Features**

### **Feature 1: Conditional Stack Tracking**

```c
// main.c
int main(int argc, char** argv) {
    error_stack_init();
    
    // Disable in release builds for performance
    #ifdef NDEBUG
        error_stack_enable(false);
    #endif
    
    // Or disable at runtime
    if (getenv("NO_ERROR_STACK")) {
        error_stack_enable(false);
    }
    
    // Run program...
}
```

### **Feature 2: Error Stack Iteration**

```c
// error.h
typedef void (*error_callback_fn)(const ErrorFrame* frame, void* user_data);

void error_stack_foreach(error_callback_fn callback, void* user_data);

// error.c
void error_stack_foreach(error_callback_fn callback, void* user_data) {
    for (int i = 0; i < g_error_stack.count; i++) {
        callback(&g_error_stack.frames[i], user_data);
    }
}

// Usage: Custom error logging
void log_error(const ErrorFrame* frame, void* user_data) {
    FILE* log = (FILE*)user_data;
    fprintf(log, "[%s] %s at %s:%d in %s()\n",
            ds_strerror(frame->code),
            frame->msg ? frame->msg : "",
            frame->file, frame->line, frame->func);
}

// Log all errors to file
FILE* log = fopen("errors.log", "a");
error_stack_foreach(log_error, log);
fclose(log);
```

### **Feature 3: Stack Snapshots**

```c
// error.h
typedef struct {
    ErrorFrame frames[ERROR_STACK_MAX];
    int count;
} ErrorSnapshot;

void error_stack_snapshot(ErrorSnapshot* snapshot);
void error_stack_restore(const ErrorSnapshot* snapshot);

// error.c
void error_stack_snapshot(ErrorSnapshot* snapshot) {
    memcpy(snapshot, &g_error_stack, sizeof(ErrorSnapshot));
}

void error_stack_restore(const ErrorSnapshot* snapshot) {
    memcpy(&g_error_stack, snapshot, sizeof(ErrorSnapshot));
}

// Usage: Save/restore error state
ErrorSnapshot snap;
error_stack_snapshot(&snap);

// Try something risky
DSError err = risky_operation();
if (err != DS_OK) {
    error_stack_restore(&snap);  // Undo error frames
}
```

### **Feature 4: JSON Error Output**

```c
// error.c
void error_stack_print_json(void) {
    printf("{\n");
    printf("  \"error_count\": %d,\n", g_error_stack.count);
    printf("  \"errors\": [\n");
    
    for (int i = 0; i < g_error_stack.count; i++) {
        ErrorFrame* frame = &g_error_stack.frames[i];
        printf("    {\n");
        printf("      \"code\": \"%s\",\n", ds_strerror(frame->code));
        printf("      \"message\": \"%s\",\n", frame->msg ? frame->msg : "");
        printf("      \"function\": \"%s\",\n", frame->func);
        printf("      \"file\": \"%s\",\n", frame->file);
        printf("      \"line\": %d\n", frame->line);
        printf("    }%s\n", i < g_error_stack.count - 1 ? "," : "");
    }
    
    printf("  ]\n");
    printf("}\n");
}
```

---

## **Testing with Error Stack**

```c
// test_framework.h - Add error stack assertions

#define ASSERT_ERROR(expr, expected_code) do {                        \
    error_stack_clear();                                              \
    DSError _err = (expr);                                            \
    if (_err != (expected_code)) {                                    \
        fprintf(stderr, "%s✗ ASSERTION FAILED%s: expected %s, got %s\n", \
                COLOR_RED, COLOR_RESET,                               \
                ds_strerror(expected_code), ds_strerror(_err));       \
        error_stack_print();                                          \
        return 1;                                                     \
    }                                                                 \
} while(0)

#define ASSERT_NO_ERROR(expr) ASSERT_ERROR(expr, DS_OK)
```

**Usage:**

```c
TEST(arena_overflow) {
    Arena* arena = arena_create(100);
    
    u8* ptr1;
    ASSERT_NO_ERROR(arena_alloc(arena, 50, &ptr1));
    
    u8* ptr2;
    ASSERT_ERROR(arena_alloc(arena, 60, &ptr2), DS_ERR_ARENA_FULL);
    
    // Check stack has one frame
    ASSERT_EQ(error_stack_count(), 1);
    
    arena_release(arena);
    return 0;
}

TEST(vector_error_propagation) {
    genVec* vec = genVec_init(1, sizeof(int), NULL, NULL, NULL);
    
    error_stack_clear();
    
    // This should fail on second push (capacity = 1)
    int val = 42;
    genVec_push(vec, cast(val));  // OK
    
    // Simulate OOM by making realloc fail (you'd need to mock this)
    // For now, just test normal propagation
    DSError err = genVec_push(vec, cast(val));
    
    if (err != DS_OK) {
        // Should have multiple frames from propagation
        ASSERT(error_stack_count() > 1);
        error_stack_print();  // See the beautiful trace!
    }
    
    genVec_destroy(vec);
    return 0;
}
```

---

## **Performance Considerations**

The error stack adds minimal overhead:

- **When disabled:** Zero overhead (just a branch check)
- **When enabled:** ~100-200 CPU cycles per error (negligible)
- **Memory:** ~4KB for the stack (32 frames × ~128 bytes each)

You can measure it:

```c
// bench_errors.c
BENCHMARK("function_without_error_stack", 1000000, {
    error_stack_enable(false);
    u8* ptr;
    arena_alloc(arena, 64, &ptr);
});

BENCHMARK("function_with_error_stack", 1000000, {
    error_stack_enable(true);
    u8* ptr;
    arena_alloc(arena, 64, &ptr);
});

// Difference should be < 5%
```

---

## **Complete Example**

```c
// main.c
#include "error.h"
#include "arena.h"
#include "gen_vector.h"

DSError process_data(void) {
    Arena* arena = arena_create(nKB(4));
    CHECK_ERROR(!arena, DS_ERR_OUT_OF_MEMORY, "failed to create arena");
    
    u8* buffer;
    DSError err = arena_alloc(arena, 1024, &buffer);
    PROPAGATE_ERROR(err, "failed to allocate buffer");
    
    genVec* vec = genVec_init(100, sizeof(int), NULL, NULL, NULL);
    CHECK_ERROR(!vec, DS_ERR_OUT_OF_MEMORY, "failed to create vector");
    
    for (int i = 0; i < 1000; i++) {
        err = genVec_push(vec, cast(i));
        PROPAGATE_ERROR(err, "failed to push data");
    }
    
    genVec_destroy(vec);
    arena_release(arena);
    return DS_OK;
}

int main(void) {
    error_stack_init();
    
    DSError err = process_data();
    if (err != DS_OK) {
        printf("Operation failed!\n");
        error_stack_print();
        return 1;
    }
    
    printf("Success!\n");
    return 0;
}
```

---

## **Summary**

This error stack system gives you:

✅ **Clean API** - Just return DSError codes  
✅ **Rich debugging** - Full stack traces with context  
✅ **Flexible** - Use as much or as little as you want  
✅ **Performance** - Can disable completely  
✅ **Propagation** - Errors naturally bubble up  
✅ **Context** - Custom messages per error  
✅ **Testable** - Easy to assert on error states  
✅ **Professional** - Like real production systems  

**This is actually how many production C libraries work internally** (like SQLite's error handling, or Linux kernel's error paths). It's the sweet spot between simplicity and power! 🎯
