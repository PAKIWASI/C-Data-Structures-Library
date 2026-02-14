# WCtoolkit

A comprehensive, type-safe C toolkit providing data structures with value semantics, memory management flexibility, and performance optimizations.

This library is built around explicit ownership, predictable performance, and modularity. All containers provide clear control over element lifetime through copy, move, and delete semantics, making ownership explicit and reducing hidden memory bugs. Value semantics are preferred where possible, but pointers can be managed safely with the same ownership patterns. The library favors cache-friendly layouts, minimal pointer indirection, and configurable growth/shrink strategies. Ultimately, it is designed to give C developers a safe and composable toolkit without sacrificing the low-level control and predictability that C code demands.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Quick Start](#quick-start)
- [Core Components](#core-components)
  - [Arena Allocator](#arena-allocator)
  - [Generic Vector](#generic-vector)
  - [String](#string)
  - [Stack](#stack)
  - [Queue](#queue)
  - [HashMap](#hashmap)
  - [HashSet](#hashset)
  - [Matrix](#matrix)
  - [Bit Vector](#bit-vector)
- [Design Philosophy](#design-philosophy)
- [Performance Considerations](#performance-considerations)
- [Examples](#examples)
- [License](#license)

---

## Overview

This library provides production-ready data structures for C with a focus on:
- **Value semantics** - Elements are stored inline with user-controlled copy/move behavior
- **Memory safety** - Comprehensive error checking and configurable allocation strategies
- **Performance** - Cache-friendly layouts and optimized algorithms
- **Flexibility** - Arena allocation support, stack/heap allocation options, and customizable callbacks

---

## Features

### Memory Management
- **Arena allocator** with scratch regions and automatic cleanup
- **Dual allocation modes** - heap or stack-based containers
- **Copy/Move semantics** - Fine-grained control over resource ownership
- **Custom destructors** - Automatic cleanup of owned resources

### Type Safety
- **Generic containers** via callback functions (no unsafe macros)
- **Generic matrix** via compile-time macros (optional)
- **Compile-time type checking** for matrix operations

### Performance
- **Cache-optimized** layouts for vectors, matrices, and hash tables
- **Blocked algorithms** for matrix operations
- **Prime-number capacities** for hash tables to reduce collisions
- **Dynamic resizing** with configurable growth/shrink factors

### Error Handling
- **Three-level diagnostics**: FATAL (exit), WARN (continue), LOG (info)
- **Assertions** with formatted messages
- **Null-pointer guards** throughout the API

---

## Quick Start

### Basic Setup

```c
#include "gen_vector.h"
#include "String.h"
#include "hashmap.h"
#include "arena.h"

int main(void) {
    // Create a vector of integers
    genVec* vec = genVec_init(10, sizeof(int), NULL, NULL, NULL);
    
    int x = 42;
    genVec_push(vec, (u8*)&x);
    
    int result;
    genVec_get(vec, 0, (u8*)&result);
    printf("Value: %d\n", result);
    
    genVec_destroy(vec);
    return 0;
}
```

### Arena-Based Allocation

```c
// Create arena with 4KB
Arena* arena = arena_create(nKB(4));

// Allocate from arena
int* nums = ARENA_ALLOC_N(arena, int, 100);
String* str = ARENA_ALLOC(arena, String);

// Use scratch region for temporary work
ARENA_SCRATCH(scratch, arena) {
    char* temp = ARENA_ALLOC_N(arena, char, 256);
    // temp is automatically freed when scope exits
}

// Release all arena memory at once
arena_release(arena);
```

---

## Core Components

### Arena Allocator

Fast, region-based memory allocator with automatic cleanup.

#### Features
- O(1) allocation with pointer bumping
- Configurable alignment (default 8 bytes)
- Scratch regions with automatic rollback
- Stack or heap initialization

#### API

```c
// Creation
Arena* arena = arena_create(nKB(4));           // 4KB heap arena
Arena stack_arena;
ARENA_CREATE_STK_ARR(&stack_arena, 4);         // 4KB stack arena

// Allocation
void* ptr = arena_alloc(arena, 256);           // 256 bytes, default alignment
void* aligned = arena_alloc_aligned(arena, 256, 16);  // 16-byte aligned

// Typed allocation
int* num = ARENA_ALLOC(arena, int);
char* buf = ARENA_ALLOC_N(arena, char, 1024);
struct Foo* foo = ARENA_ALLOC_ZERO(arena, struct Foo);

// Scratch regions (manual)
u64 mark = arena_get_mark(arena);
// ... temporary work ...
arena_clear_mark(arena, mark);

// Scratch regions (automatic)
ARENA_SCRATCH(scratch, arena) {
    // Allocations here are automatically freed
    char* temp = ARENA_ALLOC_N(arena, char, 512);
}

// Cleanup
arena_clear(arena);      // Reset, keep memory
arena_release(arena);    // Free all memory
```

#### Use Cases
- Temporary computations
- Per-frame allocations in game loops
- Fast parser/compiler allocation
- Batch processing

---

### Generic Vector

Dynamic array with value semantics and customizable element management.

#### Features
- Automatic growth/shrink (configurable factors)
- Copy, move, or POD semantics
- Stack or heap allocation
- O(1) amortized push/pop

#### API

```c
// Callbacks for custom types
void copy_string(u8* dest, const u8* src) {
    String* d = (String*)dest;
    String* s = (String*)src;
    string_copy(d, s);
}

void delete_string(u8* elem) {
    string_destroy_stk((String*)elem);
}

// Creation
genVec* vec = genVec_init(10, sizeof(String), copy_string, NULL, delete_string);

// Stack-based vector
genVec stack_vec;
genVec_init_stk(10, sizeof(int), NULL, NULL, NULL, &stack_vec);

// Operations
String s = string_from_cstr("hello");
genVec_push(vec, (u8*)&s);                    // Copy
genVec_push_move(vec, (u8**)&s);              // Move (nulls original)

String result;
genVec_get(vec, 0, (u8*)&result);             // Copy to result
const String* ptr = (String*)genVec_get_ptr(vec, 0);  // Direct access

genVec_insert(vec, 5, (u8*)&s);               // Insert at index
genVec_remove(vec, 5, (u8*)&result);          // Remove, copy out

// Batch operations
String arr[10];
genVec_insert_multi(vec, 0, (u8*)arr, 10);    // Insert array

// Utilities
if (!genVec_empty(vec)) {
    u64 size = genVec_size(vec);
    u64 cap = genVec_capacity(vec);
}

genVec_clear(vec);       // Remove all, keep capacity
genVec_reset(vec);       // Remove all, free memory
genVec_destroy(vec);     // Free everything
```

#### Configuration

```c
#define GENVEC_GROWTH 1.5F        // Capacity multiplier (default 1.5)
#define GENVEC_SHRINK_AT 0.25F    // Load factor to trigger shrink
#define GENVEC_SHRINK_BY 0.5F     // Shrink divisor
```

---

### String

Dynamic string with length tracking (not null-terminated internally).

#### Features
- Wrapper around `genVec` for `char` type
- Efficient concatenation and insertion
- Built-in search and substring operations
- Conversion to/from C strings

#### API

```c
// Creation
String* str = string_create();
String* str2 = string_from_cstr("Hello, World!");
String* copy = string_from_string(str2);

// Stack allocation
String stack_str;
string_create_stk(&stack_str, "Stack string");

// Modification
string_append_cstr(str, " more text");
string_append_string(str, str2);
string_append_char(str, '!');

string_insert_cstr(str, 0, "Start: ");
string_insert_char(str, 5, 'X');

char c = string_pop_char(str);
string_remove_char(str, 10);
string_remove_range(str, 5, 10);    // Remove [5,10] inclusive

// Access
char ch = string_char_at(str, 0);
string_set_char(str, 0, 'H');
char* data = string_data_ptr(str);  // Not null-terminated!

// Conversion
char* cstr = string_to_cstr(str);   // Malloc'd, null-terminated
// ... use cstr ...
free(cstr);

// Search
u64 pos = string_find_char(str, 'e');
u64 idx = string_find_cstr(str, "World");
if (idx != (u64)-1) {
    printf("Found at %lu\n", idx);
}

String* sub = string_substr(str, 0, 5);  // Extract substring

// Comparison
if (string_equals(str, str2)) { /*...*/ }
if (string_equals_cstr(str, "test")) { /*...*/ }

// Utilities
u64 len = string_len(str);
b8 empty = string_empty(str);

string_print(str);  // Prints "content"
string_destroy(str);
```

---

### Stack

LIFO container, implemented as a thin wrapper around `genVec`.

#### API

```c
// Creation
Stack* stk = stack_create(10, sizeof(int), NULL, NULL, NULL);

// Operations
int x = 42;
stack_push(stk, (u8*)&x);
stack_push_move(stk, (u8**)&ptr);  // Move semantics

int result;
stack_pop(stk, (u8*)&result);      // Pop and copy
stack_peek(stk, (u8*)&result);     // Peek without removing

const int* top = (int*)stack_peek_ptr(stk);

// Utilities
u64 size = stack_size(stk);
if (!stack_empty(stk)) { /*...*/ }

stack_clear(stk);
stack_destroy(stk);
```

---

### Queue

Circular FIFO queue with automatic growth/shrink.

#### Features
- Circular buffer implementation
- Dynamic resizing (grows at 150%, shrinks at 25% load)
- O(1) enqueue/dequeue
- Efficient memory usage

#### API

```c
// Creation
Queue* q = queue_create(10, sizeof(int), NULL, NULL, NULL);
Queue* q2 = queue_create_val(10, (u8*)&init_val, sizeof(int), NULL, NULL, NULL);

// Operations
int x = 42;
enqueue(q, (u8*)&x);
enqueue_move(q, (u8**)&ptr);

int result;
dequeue(q, (u8*)&result);
queue_peek(q, (u8*)&result);

const int* front = (int*)queue_peek_ptr(q);

// Utilities
u64 size = queue_size(q);
u64 cap = queue_capacity(q);
if (queue_empty(q)) { /*...*/ }

queue_shrink_to_fit(q);  // Manual shrink
queue_clear(q);
queue_destroy(q);
```

---

### HashMap

Hash table with linear probing, dynamic resizing, and custom hash/compare functions.

#### Features
- Open addressing with linear probing
- Prime-number capacities for better distribution
- Load factor triggers: 70% grow, 20% shrink
- FNV-1a hash by default
- Copy and move semantics for keys/values

#### API

```c
// Custom hash function (optional)
u64 my_hash(const u8* key, u64 size) {
    // Custom hash implementation
}

// Custom compare (optional, default is memcmp)
int my_compare(const u8* a, const u8* b, u64 size) {
    // Return 0 if equal
}

// Creation
hashmap* map = hashmap_create(
    sizeof(int),           // key size
    sizeof(String),        // value size
    NULL,                  // hash_fn (NULL = use FNV-1a)
    NULL,                  // compare_fn (NULL = memcmp)
    NULL,                  // key_copy_fn
    copy_string,           // val_copy_fn
    NULL,                  // key_move_fn
    NULL,                  // val_move_fn
    NULL,                  // key_del_fn
    delete_string          // val_del_fn
);

// Insertion (copy semantics)
int key = 42;
String val = string_from_cstr("value");
b8 updated = hashmap_put(map, (u8*)&key, (u8*)&val);

// Insertion (move semantics)
String* val_ptr = string_from_cstr("moved value");
hashmap_put_move(map, (u8**)&key_ptr, (u8**)&val_ptr);  // Both nulled

// Mixed semantics
hashmap_put_key_move(map, (u8**)&key_ptr, (u8*)&val);
hashmap_put_val_move(map, (u8*)&key, (u8**)&val_ptr);

// Retrieval
String result;
if (hashmap_get(map, (u8*)&key, (u8*)&result)) {
    printf("Found!\n");
}

String* ptr = (String*)hashmap_get_ptr(map, (u8*)&key);  // Direct access

// Deletion
String deleted;
if (hashmap_del(map, (u8*)&key, (u8*)&deleted)) {
    // Key was found and removed, value copied to 'deleted'
}

// Utilities
if (hashmap_has(map, (u8*)&key)) { /*...*/ }
u64 size = hashmap_size(map);
u64 cap = hashmap_capacity(map);

hashmap_destroy(map);
```

#### For String Keys

Use the `murmurhash3_str` function from `str_setup.h`:

```c
#include "str_setup.h"

hashmap* map = hashmap_create(
    sizeof(String), sizeof(int),
    murmurhash3_str,  // String hash function
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
);

String key = string_from_cstr("mykey");
int val = 100;
hashmap_put(map, (u8*)&key, (u8*)&val);
```

---

### HashSet

Hash set for unique elements with same implementation strategy as HashMap.

#### API

```c
// Creation
hashset* set = hashset_create(
    sizeof(int),      // element size
    NULL,             // hash_fn
    NULL,             // compare_fn
    NULL,             // copy_fn
    NULL,             // move_fn
    NULL              // del_fn
);

// Insertion
int x = 42;
b8 existed = hashset_insert(set, (u8*)&x);  // Returns 1 if already exists

int* ptr = &x;
hashset_insert_move(set, (u8**)&ptr);  // Move semantics, nulls ptr

// Operations
if (hashset_has(set, (u8*)&x)) {
    printf("Set contains %d\n", x);
}

b8 removed = hashset_remove(set, (u8*)&x);

// Utilities
u64 size = hashset_size(set);
hashset_clear(set);   // Remove all, keep capacity
hashset_reset(set);   // Remove all, reset to initial capacity
hashset_destroy(set);
```

---

### Matrix

Row-major 2D matrix with optimized operations for numerical computing.

#### Features
- Blocked matrix multiplication for cache efficiency
- Two multiplication variants (standard and transpose-optimized)
- LU decomposition and determinant calculation
- Arena allocation support

#### API

```c
// Creation
Matrix* mat = matrix_create(3, 3);                    // 3x3 matrix
Matrix* mat2 = matrix_create_arr(3, 3, (float[9]){   // With initial values
    1, 2, 3,
    4, 5, 6,
    7, 8, 9
});

// Stack allocation
Matrix stack_mat;
float data[9];
matrix_create_stk(&stack_mat, 3, 3, data);

// Arena allocation
Matrix* arena_mat = matrix_arena_alloc(arena, 3, 3);
Matrix* arena_mat2 = matrix_arena_arr_alloc(arena, 3, 3, (float[9]){
    1, 2, 3, 4, 5, 6, 7, 8, 9
});

// Setting values
matrix_set_val_arr(mat, 9, (float*)(float[3][3]){
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
});

matrix_set_elm(mat, 3.14f, 0, 0);  // Set element at (0,0)

// Direct access
float val = MATRIX_AT(mat, 0, 0);  // Get element at (0,0)
MATRIX_AT(mat, 1, 2) = 42.0f;      // Set element at (1,2)

// Basic operations
Matrix* sum = matrix_create(3, 3);
matrix_add(sum, mat, mat2);        // sum = mat + mat2
matrix_sub(sum, mat, mat2);        // sum = mat - mat2

matrix_scale(mat, 2.0f);           // mat *= 2.0
matrix_div(mat, 2.0f);             // mat /= 2.0

Matrix* dest = matrix_create(3, 3);
matrix_copy(dest, mat);            // Deep copy

// Matrix multiplication
Matrix* a = matrix_create(3, 4);   // 3x4
Matrix* b = matrix_create(4, 5);   // 4x5
Matrix* c = matrix_create(3, 5);   // 3x5 result

matrix_xply(c, a, b);              // Blocked ikj multiplication
matrix_xply_2(c, a, b);            // Transpose-optimized (for large matrices)

// Transpose
Matrix* t = matrix_create(3, 3);
Matrix* mat_t = matrix_create(3, 3);
matrix_T(mat_t, mat);              // mat_t = mat^T

// LU Decomposition
Matrix* L = matrix_create(3, 3);
Matrix* U = matrix_create(3, 3);
matrix_LU_Decomp(L, U, mat);       // mat = L * U

// Determinant
float det = matrix_det(mat);

// Utilities
matrix_print(mat);
u64 total = MATRIX_TOTAL(mat);     // Total elements (m * n)

matrix_destroy(mat);
```

#### Generic Matrix (Compile-Time)

For type-generic matrices with compile-time safety:

```c
#include "matrix_generic.h"

// Instantiate matrix type for int
INSTANTIATE_MATRIX(int, "%d ");

// Now you can use:
Matrix_int* mat = matrix_create_int(3, 3);
matrix_set_elm_int(mat, 42, 0, 0);
matrix_print_int(mat);
double det = matrix_det_int(mat);  // Returns double
matrix_destroy_int(mat);
```

Supported operations for generic matrices:
- `matrix_create_T`, `matrix_create_arr_T`, `matrix_create_stk_T`
- `matrix_add_T`, `matrix_sub_T`, `matrix_scale_T`, `matrix_div_T`
- `matrix_xply_T`, `matrix_xply_2_T`, `matrix_T_T`
- `matrix_LU_Decomp_T`, `matrix_det_T`

---

### Bit Vector

Compact bit storage with dynamic sizing.

#### API

```c
// Creation
bitVec* bv = bitVec_create();

// Operations
bitVec_set(bv, 42);        // Set bit 42 to 1
bitVec_clear(bv, 42);      // Set bit 42 to 0
bitVec_toggle(bv, 42);     // Flip bit 42

u8 bit = bitVec_test(bv, 42);  // Get bit 42 (0 or 1)

bitVec_push(bv);           // Append a bit (set to 1)
bitVec_pop(bv);            // Remove last bit

// Utilities
u64 bits = bitVec_size_bits(bv);    // Number of bits
u64 bytes = bitVec_size_bytes(bv);  // Number of bytes

bitVec_print(bv, 0);       // Print byte 0
bitVec_destroy(bv);
```

---

## Design Philosophy

### Value Semantics

All containers store elements **by value** (inline), not by pointer. This:
- Eliminates pointer-chasing overhead
- Improves cache locality
- Avoids ownership ambiguity
- Reduces allocations

### Copy vs Move

The library distinguishes between:
- **Copy operations** (`_push`, `_insert`, etc.) - Deep copy via callbacks
- **Move operations** (`_push_move`, `_insert_move`) - Transfer ownership, null source

Example:
```c
String* s = string_from_cstr("hello");

// Copy: s remains valid
genVec_push(vec, (u8*)&s);
string_destroy(s);  // Still need to free

// Move: s is nulled
String* s2 = string_from_cstr("world");
genVec_push_move(vec, (u8**)&s2);
// s2 is now NULL, no need to destroy
```

### Memory Management Patterns

#### POD Types (Plain Old Data)
```c
// No callbacks needed for simple types
genVec* ints = genVec_init(10, sizeof(int), NULL, NULL, NULL);
```

#### Types with Resources
```c
// Provide copy and delete callbacks
genVec* strings = genVec_init(10, sizeof(String), 
    copy_string,    // Deep copy
    move_string,    // Move (optional)
    delete_string   // Cleanup
);
```

#### Arena Pattern
```c
Arena* arena = arena_create(nMB(1));

// All allocations from arena
Matrix* matrices[100];
for (int i = 0; i < 100; i++) {
    matrices[i] = matrix_arena_alloc(arena, 10, 10);
}

// Single cleanup
arena_release(arena);  // Frees all 100 matrices
```

---

## Performance Considerations

### Cache Optimization

**Vectors and Queues**
- Contiguous memory layout
- Minimal pointer indirection
- Cache-friendly iteration

**Matrices**
- Row-major storage for predictable access patterns
- Blocked algorithms (16x16 tiles by default)
- Two multiplication strategies:
  - `matrix_xply`: In-place blocked ikj (small/medium matrices)
  - `matrix_xply_2`: Transpose B first (large matrices, more memory)

**Hash Tables**
- Prime capacities reduce clustering
- Linear probing for cache locality
- Tombstone states for deletion

### Growth Strategies

**Vectors** (configurable):
- Growth: Capacity × 1.5 when full
- Shrink: Capacity × 0.5 when 25% full

**Queues**:
- Growth: Capacity × 1.5 when full
- Shrink: Capacity × 0.5 when 25% full
- Minimum capacity: 4

**Hash Tables**:
- Growth: Next prime when load > 70%
- Shrink: Previous prime when load < 20%
- Initial capacity: 17

### Alignment

Default alignment is 8 bytes (`sizeof(u64)`). Customize:

```c
#define ARENA_DEFAULT_ALIGNMENT 16  // Before including arena.h
#include "arena.h"

// Or use aligned allocation
void* ptr = arena_alloc_aligned(arena, 256, 64);  // 64-byte aligned
```

---

## Examples

### Example 1: Basic String Operations

```c
#include "String.h"

int main(void) {
    // Heap-allocated string
    String* s1 = string_create(); 
    string_append_cstr(s1, "this is s1");
    string_print(s1);  // Output: "this is s1"
    printf("\n");
    string_destroy(s1);

    // Stack-allocated string with heap data
    String s2;
    string_create_stk(&s2, "this is s2");
    string_print(&s2);  // Output: "this is s2"
    printf("\n");
    string_destroy_stk(&s2);

    // Create from C string
    String* s3 = string_from_cstr("this is s3");
    string_print(s3);
    printf("\n");
    
    // Copy constructor
    String* s4 = string_from_string(s3);
    string_destroy(s3);

    // Reserve with character fill
    string_reserve_char(s4, 20, 'x');
    string_print(s4);  // Output: "this is s3xxxxxxxxxx..."
    printf("\n");
    
    // Deep copy to stack string
    String s5;
    string_copy(&s5, s4);
    string_destroy(s4);

    string_print(&s5);
    printf("\n");
    string_destroy_stk(&s5);

    return 0;
}
```

### Example 2: Generic Vector - Copy vs Move Semantics

```c
#include "gen_vector.h"
#include "String.h"

// Helper callbacks (see helpers.h for implementation)
void str_copy(u8* dest, const u8* src);
void str_move(u8* dest, u8** src);
void str_del(u8* elm);
void str_print(const u8* elm);

int main(void) {
    // Vector storing String by value (not pointer)
    genVec* vec = genVec_init(10, sizeof(String), str_copy, str_move, str_del);

    // Copy semantics - original remains valid
    String* str = string_from_cstr("hello");
    genVec_push(vec, (u8*)str);
    genVec_push(vec, (u8*)str);

    string_append_cstr(str, " what is up");
    genVec_push(vec, (u8*)str);
    
    string_print(str);      // Still valid
    string_destroy(str);    // Must free original

    // Deep copy entire vector
    genVec v2;
    genVec_copy(&v2, vec);

    genVec_print(vec, str_print);
    genVec_print(&v2, str_print);
    
    genVec_destroy(vec);
    genVec_destroy_stk(&v2);

    return 0;
}

// Move semantics example
int move_example(void) {
    genVec* vec = genVec_init(10, sizeof(String), str_copy, str_move, str_del);

    String* str = string_from_cstr("hello");
    string_print(str);
    printf("\n");

    // Move - transfers ownership, nulls original
    genVec_push_move(vec, (u8**)&str);
    // str is now NULL, no need to destroy

    // Move entire vector to stack variable
    genVec v2;
    genVec_move(&v2, &vec);  // vec is now NULL

    genVec_print(&v2, str_print);
    genVec_destroy_stk(&v2);

    return 0;
}
```

### Example 3: HashMap with Integer Keys and String Values

```c
#include "hashmap.h"
#include "String.h"

// Helper callbacks
void str_copy(u8* dest, const u8* src);
void str_move(u8* dest, u8** src);
void str_del(u8* elm);
void str_print(const u8* elm);
void int_print(const u8* elm);

int main(void) {
    // Create map: int -> String
    hashmap* map = hashmap_create(
        sizeof(int), sizeof(String),
        NULL, NULL,              // Use default hash and compare
        NULL, str_copy,          // key copy, val copy
        NULL, str_move,          // key move, val move
        NULL, str_del            // key del, val del
    );

    // Insert with copy semantics
    int a = 5;
    String str;
    string_create_stk(&str, "hello");

    hashmap_put(map, (u8*)&a, (u8*)&str);
    a++;
    hashmap_put(map, (u8*)&a, (u8*)&str);
    a++;
    hashmap_put(map, (u8*)&a, (u8*)&str);

    hashmap_print(map, int_print, str_print);

    // Get pointer to value and modify in-place
    String* s = (String*)hashmap_get_ptr(map, (u8*)&a);
    string_append_cstr(s, " what is up");
    string_print(s);
    printf("\n");

    hashmap_print(map, int_print, str_print);
    
    // Get copy of value
    String s2 = {0};
    hashmap_get(map, (u8*)&a, (u8*)&s2);
    string_print(&s2);
    printf("\n");

    // Delete entry
    hashmap_del(map, (u8*)&a, NULL);
    printf("Has key %d: %d\n", a, hashmap_has(map, (u8*)&a));

    string_destroy_stk(&str);
    string_destroy_stk(&s2);
    hashmap_destroy(map);
    return 0;
}
```

### Example 4: HashMap with Move Semantics

```c
#include "hashmap.h"
#include "String.h"

int main(void) {
    hashmap* map = hashmap_create(
        sizeof(int), sizeof(String),
        NULL, NULL, NULL, str_copy, NULL, str_move, NULL, str_del
    );

    // Insert with move semantics (value)
    int a = 7;
    String* str = string_from_cstr("hello");
    hashmap_put_val_move(map, (u8*)&a, (u8**)&str);  // str is now NULL

    str = string_from_cstr("what is up");
    a += 2;
    hashmap_put_val_move(map, (u8*)&a, (u8**)&str);
    
    hashmap_print(map, int_print, str_print);

    // Delete and retrieve value
    String s;
    hashmap_del(map, (u8*)&a, (u8*)&s);
    string_print(&s);
    printf("\n");

    hashmap_print(map, int_print, str_print);
    printf("Size: %lu\n", hashmap_size(map));
    
    string_destroy_stk(&s);
    hashmap_destroy(map);
    return 0;
}
```

### Example 5: HashMap - String Keys to String Values

```c
#include "hashmap.h"
#include "String.h"
#include "str_setup.h"  // For murmurhash3_str

// String comparison function
int str_cmp(const u8* a, const u8* b, u64 size) {
    (void)size;
    return string_compare((const String*)a, (const String*)b);
}

// Helper to insert string pairs with move semantics
void map_put(hashmap* map, const char* k, const char* v) {
    String* s1 = string_from_cstr(k);
    String* s2 = string_from_cstr(v);
    hashmap_put_move(map, (u8**)&s1, (u8**)&s2);
}

int main(void) {
    // Create map: String -> String
    hashmap* map = hashmap_create(
        sizeof(String), sizeof(String), 
        murmurhash3_str,     // Custom hash for String keys
        str_cmp,             // String comparison
        str_copy, str_copy,  // Copy functions
        str_move, str_move,  // Move functions
        str_del, str_del     // Delete functions
    );

    map_put(map, "what", "up");

    String s1;
    string_create_stk(&s1, "what");

    // Get pointer and modify in-place
    String* val = (String*)hashmap_get_ptr(map, (u8*)&s1);
    string_append_cstr(val, "__hi");
    
    // Update the map (rehashes with new value)
    hashmap_put(map, (u8*)val, (u8*)val); 

    hashmap_print(map, str_print, str_print);

    string_destroy_stk(&s1);
    hashmap_destroy(map);
    return 0;
}
```

### Example 6: HashMap - Storing Vectors as Values

```c
#include "hashmap.h"
#include "gen_vector.h"

// Vector callbacks for hashmap storage
void vec_copy(u8* dest, const u8* src) {
    genVec* s = (genVec*)src; 
    genVec* d = (genVec*)dest; 

    memcpy(d, s, sizeof(genVec));
    d->data = malloc(s->capacity * (u64)s->data_size);

    if (s->copy_fn) {
        for (u64 i = 0; i < s->size; i++) {
            s->copy_fn(d->data + (i * d->data_size), genVec_get_ptr(s, i));
        }
    } else {
        memcpy(d->data, s->data, s->capacity * (u64)s->data_size);
    }
}

void vec_del(u8* elm) {
    genVec_destroy_stk((genVec*)elm); 
}

void vec_print(const u8* elm) {
    genVec* v = (genVec*)elm;
    printf("[");
    for (u64 i = 0; i < v->size; i++) {
        printf("%d ", *(int*)genVec_get_ptr(v, i));
    }
    printf("]");
}

int main(void) {
    hashmap* map = hashmap_create(
        sizeof(u32), sizeof(genVec), 
        NULL, NULL, NULL, vec_copy, NULL, NULL, NULL, vec_del
    );

    u32 a = 10;
    genVec* vec = genVec_init_val(10, (u8*)&a, sizeof(u32), NULL, NULL, NULL);

    // Copy vector into map
    hashmap_put(map, (u8*)&a, (u8*)vec);
    hashmap_print(map, int_print, vec_print);

    // Create new vector with different values
    a = 2;
    genVec_reserve_val(vec, 100, (u8*)&a);

    // Move vector into map
    hashmap_put_val_move(map, (u8*)&a, (u8**)&vec);
    hashmap_print(map, int_print, vec_print);

    // Delete and retrieve
    genVec* v = malloc(sizeof(genVec));
    hashmap_del(map, (u8*)&a, (u8*)v);
    
    genVec_print(v, int_print);

    genVec_destroy(v);
    hashmap_destroy(map);
    return 0;
}
```

### Example 7: HashSet with String Elements

```c
#include "hashset.h"
#include "String.h"
#include "str_setup.h"

int main(void) {
    hashset* set = hashset_create(
        sizeof(String), 
        murmurhash3_str, str_cmp, 
        str_copy, str_move, str_del
    );

    // Insert with move semantics
    String* s1 = string_from_cstr("hello");
    String* s2 = string_from_cstr("hollo");

    hashset_insert_move(set, (u8**)&s1);  // s1 is now NULL
    hashset_insert_move(set, (u8**)&s2);  // s2 is now NULL

    hashset_print(set, str_print);

    // Conditional insert/remove
    String* s3 = string_from_cstr("helllo");
    if (!hashset_remove(set, (u8*)s3)) {
        hashset_insert(set, (u8*)s3);
    } 
    hashset_print(set, str_print);

    // Try remove again
    if (!hashset_remove(set, (u8*)s3)) {
        hashset_insert_move(set, (u8**)&s3);
    } else {
        string_destroy(s3);
    }

    hashset_print(set, str_print);

    hashset_destroy(set);
    return 0;
}
```

### Example 8: Matrix Operations and LU Decomposition

```c
#include "matrix.h"

int main(void) {
    // Create matrix with initial values (preferred syntax)
    Matrix* mat = matrix_create_arr(3, 3, (float*)(float[3][3]){
        {3, 2, 4},
        {2, 0, 2},
        {4, 2, 3},
    });

    // Create stack matrices for decomposition
    Matrix L, U;
    matrix_create_stk(&L, 3, 3, (float*)ZEROS_2D(3, 3));
    matrix_create_stk(&U, 3, 3, (float*)ZEROS_2D(3, 3));

    // Perform LU decomposition
    matrix_LU_Decomp(&L, &U, mat);

    printf("L matrix:\n");
    matrix_print(&L);
    
    printf("U matrix:\n");
    matrix_print(&U);

    // Calculate determinant
    float det = matrix_det(mat);
    printf("Det: %f\n", det);

    // Copy matrix
    Matrix copy;
    matrix_create_stk(&copy, 3, 3, (float*)ZEROS_2D(3, 3));
    matrix_copy(&copy, mat);
    matrix_print(&copy);

    matrix_destroy(mat);
    return 0;
}
```

### Example 9: Matrix Multiplication and Transpose

```c
#include "matrix.h"

int main(void) {
    Matrix* m1 = matrix_create_arr(4, 3, (float*)(float[4][3]){
        {1, 2, 3},
        {2, 2, 7},
        {1, 0, 1},
        {8, 9, 3},
    });

    Matrix m2;
    matrix_create_stk(&m2, 3, 4, (float*)(float[3][4]){
        {1, 2, 3, 4},
        {1, 0, 9, 6},
        {8, 4, 0, 4},
    });

    // Matrix multiplication: (4x3) × (3x4) = (4x4)
    Matrix out;
    matrix_create_stk(&out, 4, 4, (float*)ZEROS_2D(4, 4));
    matrix_xply(&out, m1, &m2);
    matrix_print(&out);

    // Transpose
    Matrix out_T;
    matrix_create_stk(&out_T, 3, 4, (float*)ZEROS_2D(3, 4));
    matrix_T(&out_T, m1);
    
    printf("Original:\n");
    matrix_print(m1);
    printf("Transposed:\n");
    matrix_print(&out_T);

    matrix_destroy(m1);
    return 0;
}
```

### Example 10: Arena with Matrix Scratch Space

```c
#include "arena.h"
#include "matrix.h"

int main(void) {
    Arena* arena = arena_create(nKB(1));

    // Result matrix from arena
    Matrix* mat = matrix_arena_alloc(arena, 4, 4);

    // Temporary matrices in scratch region
    ARENA_SCRATCH(xplyy, arena) {
        Matrix* t1 = matrix_arena_arr_alloc(arena, 4, 4, (float*)(float[4][4]){
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4}
        });

        Matrix* t2 = matrix_arena_arr_alloc(arena, 4, 4, (float*)(float[4][4]){
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4},
            {1, 2, 3, 4}
        });

        // Compute result
        matrix_xply(mat, t1, t2);
        
    }  // t1 and t2 automatically freed here

    // mat is still valid
    matrix_print(mat);

    printf("Used: %lu / Total: %lu\n", arena->idx, arena->size);
    
    // Single cleanup
    arena_release(arena);
    return 0;
}
```

### Example 11: Arena Scratch Regions

```c
#include "arena.h"

int main(void) {
    Arena* arena = arena_create(nKB(4));

    // Automatic scratch region (preferred)
    ARENA_SCRATCH(temp, arena) {
        u8* a = arena_alloc(arena, nKB(1));
        // Use temporary allocation...
    }  // Automatically freed here

    // Manual scratch region
    arena_scratch sc = arena_scratch_begin(arena);
    u8* a = arena_alloc(arena, nKB(2));
    // Use temporary allocation...
    arena_scratch_end(&sc);

    arena_release(arena);
    return 0;
}
```

### Example 12: Queue Operations

```c
#include "Queue.h"

void int_print(const u8* elm) {
    printf("%d", *(int*)elm);
}

int main(void) {
    Queue* q = queue_create(100, sizeof(int), NULL, NULL, NULL);

    // Enqueue elements
    int a = 5;
    enqueue(q, (u8*)&a);
    a++;
    enqueue(q, (u8*)&a);
    a++;
    enqueue(q, (u8*)&a);
    a++;
    enqueue(q, (u8*)&a);

    queue_print(q, int_print);  // Output: [ 5 6 7 8 ]
    putchar('\n');

    // Dequeue elements
    dequeue(q, NULL);
    dequeue(q, NULL);
    dequeue(q, NULL);
    
    queue_print(q, int_print);  // Output: [ 8 ]
    putchar('\n');

    // Reset queue
    queue_reset(q);
    queue_print(q, int_print);  // Output: [ ]
    putchar('\n');

    enqueue(q, (u8*)&a);
    queue_print(q, int_print);  // Output: [ 8 ]
    putchar('\n');

    queue_destroy(q);
    return 0;
}
```

### Example 13: Stack Operations

```c
#include "Stack.h"

void int_print(const u8* elm) {
    printf("%d", *(int*)elm);
}

int main(void) {
    Stack* stk = stack_create(10, sizeof(int), NULL, NULL, NULL);

    // Push elements
    int a = 5;
    for (int i = 0; i < 9; i++) {
        stack_push(stk, (u8*)&a);
    }

    // Peek at top
    printf("Top: %d\n", *(int*)stack_peek_ptr(stk));

    stack_print(stk, int_print);
    putchar('\n');

    // Pop elements
    for (int i = 0; i < 9; i++) {
        stack_pop(stk, NULL);
    }

    stack_print(stk, int_print);
    putchar('\n');

    printf("Empty: %d\n", stack_empty(stk));

    stack_destroy(stk);
    return 0;
}
```

### Example 14: Generic Vector with Simple Macros

```c
#include "gen_vector.h"

void float_print(const u8* elm) {
    printf("%f", *(float*)elm);
}

// Macro for simple push
#define VEC_PUSH_SIMP(vec, type, val) genVec_push(vec, (u8*)&(type){val})

int main(void) {
    genVec vec;
    genVec_init_stk(1000, sizeof(float), NULL, NULL, NULL, &vec);

    // Push using macro
    for (int i = 0; i < 1000; i++) {
        VEC_PUSH_SIMP(&vec, float, (float)i + 0.1f);
    }

    genVec_print(&vec, float_print);

    genVec_destroy_stk(&vec);
    return 0;
}
```

---

## Common Patterns

### Error Handling

```c
// FATAL errors exit the program
Arena* arena = arena_create(0);  // Exits if malloc fails

// WARN errors continue execution
u8* ptr = arena_alloc(arena, TOO_MUCH);  // Returns NULL, prints warning

// Check warnings
if (!ptr) {
    // Handle error
}

// Custom checks
CHECK_FATAL(ptr == NULL, "custom error message");
CHECK_WARN(size > MAX, "size too large: %zu", size);
```

### Iteration

```c
// Vector iteration
for (u64 i = 0; i < genVec_size(vec); i++) {
    const MyType* elem = (MyType*)genVec_get_ptr(vec, i);
    // Use elem
}

// HashMap iteration (requires accessing internals)
void print_key(const u8* key) {
    printf("%d ", *(int*)key);
}

void print_val(const u8* val) {
    printf("%s\n", ((String*)val)->data);
}

hashmap_print(map, print_key, print_val);
```

### Resource Ownership

```c
// Owner: Vector manages element lifetime
genVec* vec = genVec_init(10, sizeof(String), 
                          string_copy_fn, NULL, string_delete_fn);

String s = string_from_cstr("test");
genVec_push(vec, (u8*)&s);  // Vector now owns a copy
string_destroy_stk(&s);      // Free original

genVec_destroy(vec);         // Frees all string copies

// Non-owner: Vector doesn't manage lifetime
genVec* ptrs = genVec_init(10, sizeof(String*), NULL, NULL, NULL);

String* s_ptr = string_from_cstr("test");
genVec_push(ptrs, (u8*)&s_ptr);  // Just stores pointer

genVec_destroy(ptrs);            // Doesn't free strings
string_destroy(s_ptr);           // Must free manually
```

---

## Building

### Compilation

```bash
# Compile library
gcc -c arena.c gen_vector.c String.c hashmap.c matrix.c -O3 -Wall -Wextra

# Link with your code
gcc main.c arena.o gen_vector.o String.o hashmap.o matrix.o -lm -o myprogram
```

### Configuration Macros

```c
// Before including headers:

// Arena
#define ARENA_DEFAULT_ALIGNMENT 16
#define ARENA_DEFAULT_SIZE nKB(8)

// Vector
#define GENVEC_GROWTH 2.0F
#define GENVEC_SHRINK_AT 0.20F

// HashMap
#define LOAD_FACTOR_GROW 0.75
#define LOAD_FACTOR_SHRINK 0.15
#define HASHMAP_INIT_CAPACITY 17

#include "arena.h"
#include "gen_vector.h"
#include "hashmap.h"
```

---

## API Reference Summary

### Common Types

```c
typedef uint8_t  u8;
typedef uint8_t  b8;   // Boolean: false (0), true (1)
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
```

### Error Macros

```c
FATAL(fmt, ...)              // Print and exit
WARN(fmt, ...)               // Print and continue
LOG(fmt, ...)                // Print info

CHECK_FATAL(cond, fmt, ...)  // Assert or exit
CHECK_WARN(cond, fmt, ...)   // Warn if condition
ASSERT_FATAL(cond, fmt, ...) // Assert or exit
```

### Callback Types

```c
typedef void (*genVec_copy_fn)(u8* dest, const u8* src);
typedef void (*genVec_move_fn)(u8* dest, u8** src);
typedef void (*genVec_delete_fn)(u8* elem);
typedef void (*genVec_print_fn)(const u8* elm);

typedef u64 (*custom_hash_fn)(const u8* key, u64 size);
typedef int (*compare_fn)(const u8* a, const u8* b, u64 size);
```

---

## License

MIT

---

## Contributing

Contributions welcome! Areas for improvement:
- Additional container types (deque, priority queue, trie)
- SIMD optimizations for matrix operations
- More comprehensive test suite
- Documentation improvements

---

## Notes

### Implementation Details

- **Hash tables** use open addressing with linear probing
- **Queues** use circular buffer to avoid shifting elements
- **Matrices** use blocked algorithms (16x16 default) for cache efficiency
- **Arenas** align to 8 bytes by default (configurable)

### Memory Layout

```
genVec:     | data* | size | capacity | data_size | fn_ptrs... |
Arena:      | base* | idx  | size |
Matrix:     | data* | m    | n    |
```

### Future Enhancements

- Matrix adjugate and inverse
- HashMap/HashSet iterators
- String views (zero-copy slicing)
- Small string optimization (SSO)
- Generic tree structures
- Thread-safe variants

---

## Credits

Developed with modern C best practices, focusing on:
- Zero-cost abstractions where possible
- Explicit memory management
- Cache-aware algorithms
- Minimal dependencies (only stdlib)

For questions or issues, please refer to the source code documentation in header files.
