# Generic Data Structures Library in C

A collection of high-performance, generic data structures implemented in C, including vectors, hashmaps, hashsets, binary search trees, queues, stacks, bit vectors, and dynamic strings.

## Features

- **Generic Vector (`genVec`)**: Dynamic array with automatic resizing
- **HashMap**: Hash table with linear probing and dynamic resizing
- **HashSet**: Set implementation using hash table
- **Binary Search Tree (`BST`)**: Array-based BST with bit vector for tracking occupied nodes
- **Bit Vector (`bitVec`)**: Space-efficient bit array
- **Queue**: Circular queue implementation
- **Stack**: LIFO stack wrapper around vector
- **String**: Dynamic string with rich manipulation API
- **Trie**: Prefix tree for string storage (header-only)
- Custom memory management with optional delete functions
- Type-safe generic operations using `uint8_t*` casting

## Components

### 1. Generic Vector (`genVec`)
A dynamic array that can store any data type.

**Key Features:**
- Automatic growth (1.5x) and shrinking (0.5x at 25% capacity)
- Custom delete functions for complex types
- Efficient memory management
- Rich API for insertion, removal, and access operations
- Stack allocation support with `genVec_init_stk()`

**Usage:**
```c
// Create a vector of integers
genVec* vec = genVec_init(10, sizeof(int), NULL);

// Push elements
int value = 42;
genVec_push(vec, (u8*)&value);

// Access elements
int result;
genVec_get(vec, 0, (u8*)&result);

// Stack allocation
genVec vec_stk;
genVec_init_stk(10, sizeof(int), NULL, &vec_stk);
genVec_destroy_stk(&vec_stk);

// Cleanup
genVec_destroy(vec);
```

### 2. HashMap
A hash table implementation with linear probing collision resolution.

**Key Features:**
- Dynamic resizing (grows at 70% load factor, shrinks at 20%)
- Prime number capacities for better distribution
- Tombstone marking for efficient deletion
- Custom hash, compare, and delete functions
- FNV-1a hash function by default
- In-place value modification with `hashmap_modify()`

**Usage:**
```c
// Create a hashmap with String keys and int values
hashmap* map = hashmap_create(
    sizeof(String), 
    sizeof(int),
    murmurhash3_string,  // Custom hash for String
    string_custom_delete, // Delete function for keys
    NULL,                 // No delete needed for int values
    string_custom_compare // Compare function for keys
);

// Insert key-value pair
String key;
string_create_onstack(&key, "hello");
int value = 100;
hashmap_put(map, (u8*)&key, (u8*)&value);

// Retrieve value
int retrieved;
if (hashmap_get(map, (u8*)&key, (u8*)&retrieved) == 0) {
    printf("Value: %d\n", retrieved);
}

// Cleanup
string_destroy_fromstk(&key);
hashmap_destroy(map);
```

### 3. HashSet
A set implementation for storing unique elements.

**Key Features:**
- Same collision resolution and resizing strategy as HashMap
- Efficient membership testing
- Automatic duplicate prevention
- Linear probing with tombstone marking
- Dynamic resizing based on load factor

**Usage:**
```c
// Create a hashset of integers
hashset* set = hashset_create(
    sizeof(int),
    NULL,  // Use default FNV-1a hash
    NULL,  // No custom delete needed for ints
    NULL   // Use default memcmp compare
);

// Insert elements
int value = 42;
hashset_insert(set, (u8*)&value);

// Check membership
if (hashset_has(set, (u8*)&value)) {
    printf("Element 42 found!\n");
}

// Remove element
hashset_remove(set, (u8*)&value);

// Cleanup
hashset_destroy(set);
```

### 4. Binary Search Tree (`BST`)
An array-based BST implementation using implicit heap indexing.

**Key Features:**
- Array-based storage with implicit parent-child relationships
- Bit vector to track occupied vs empty array slots
- Standard BST operations (insert, remove, search)
- Multiple traversal methods (preorder, inorder, postorder, BFS)
- Find min/max operations
- Balance operation (experimental)

**Implementation Details:**
- Parent index: `(i - 1) / 2`
- Left child: `2 * i + 1`
- Right child: `2 * i + 2`
- Uses bit vector to distinguish allocated vs occupied slots

**Usage:**
```c
// Create BST for integers
BST* bst = bst_create(
    sizeof(int),
    int_cmp,      // Comparison function
    int_to_str,   // Convert to String for traversal
    NULL          // No custom delete needed
);

// Insert elements
int values[] = {50, 30, 70, 20, 40, 60, 80};
for (int i = 0; i < 7; i++) {
    bst_insert(bst, (u8*)&values[i]);
}

// Search
int search_val = 40;
if (bst_search(bst, (u8*)&search_val)) {
    printf("Found %d\n", search_val);
}

// Traversals
String* inorder = bst_inorder(bst);
printf("Inorder: %s\n", string_to_cstr(inorder));
string_destroy(inorder);

String* bfs = bst_bfs(bst);
printf("BFS: %s\n", string_to_cstr(bfs));
string_destroy(bfs);

// Find min/max
int min, max;
bst_find_min(bst, (u8*)&min);
bst_find_max(bst, (u8*)&max);
printf("Min: %d, Max: %d\n", min, max);

// Balance (WARNING: experimental, may have issues)
bst_balance(bst);

// Cleanup
bst_destroy(bst);
```

### 5. Bit Vector (`bitVec`)
A space-efficient bit array using byte-packed storage.

**Key Features:**
- 8x memory savings compared to bool arrays
- Individual bit manipulation (set, clear, test, toggle)
- Push/pop operations
- Built on top of `genVec` for automatic resizing

**Usage:**
```c
// Create bit vector
bitVec* bvec = bitVec_create();

// Set bits
bitVec_set(bvec, 0);   // Set bit 0
bitVec_set(bvec, 5);   // Set bit 5
bitVec_set(bvec, 10);  // Set bit 10

// Test bits
if (bitVec_test(bvec, 5)) {
    printf("Bit 5 is set\n");
}

// Clear bit
bitVec_clear(bvec, 5);

// Toggle bit
bitVec_toggle(bvec, 0);

// Push a set bit
bitVec_push(bvec);

// Print byte at index
bitVec_print(bvec, 0);  // Print first byte

// Cleanup
bitVec_destroy(bvec);
```

### 6. Queue
A circular queue implementation with automatic growth.

**Key Features:**
- Circular buffer for O(1) enqueue/dequeue
- Automatic resizing (1.5x growth)
- No shrinking (for performance)
- Built on top of `genVec`

**Usage:**
```c
// Create queue of integers
Queue* q = queue_create(10, sizeof(int), NULL);

// Enqueue elements
for (int i = 0; i < 5; i++) {
    enqueue(q, (u8*)&i);
}

// Dequeue element
int val;
dequeue(q, (u8*)&val);
printf("Dequeued: %d\n", val);

// Peek at front
queue_peek(q, (u8*)&val);
printf("Front: %d\n", val);

// Check size
printf("Size: %zu\n", queue_size(q));

// Cleanup
queue_destroy(q);
```

### 7. Stack
A simple LIFO stack wrapper around `genVec`.

**Key Features:**
- Standard push/pop/peek operations
- Automatic resizing through underlying vector
- Thin wrapper for clarity

**Usage:**
```c
// Create stack of integers
Stack* stk = stack_create(10, sizeof(int), NULL);

// Push elements
for (int i = 0; i < 5; i++) {
    stack_push(stk, (u8*)&i);
}

// Pop element
int val;
stack_pop(stk, (u8*)&val);
printf("Popped: %d\n", val);

// Peek at top
stack_peek(stk, (u8*)&val);
printf("Top: %d\n", val);

// Cleanup
stack_destroy(stk);
```

### 8. Dynamic String
A robust string implementation with automatic memory management.

**Key Features:**
- Built on top of `genVec`
- Automatic null termination
- Rich string manipulation API
- Support for both heap and stack allocation
- Substring extraction
- Search operations

**Usage:**
```c
// Create from C string
String* str = string_from_cstr("Hello");

// Append operations
string_append_cstr(str, " World");
string_append_char(str, '!');

// Insert operations
string_insert_char(str, 5, ',');
string_insert_cstr(str, 6, " beautiful");

// Access and modify
char c = string_at(str, 0);
string_set_char(str, 0, 'h');

// Search
int pos = string_find_cstr(str, "World");
if (pos != -1) {
    printf("Found at position %d\n", pos);
}

// Substring
String* sub = string_substr(str, 0, 5);
printf("Substring: %s\n", string_to_cstr(sub));
string_destroy(sub);

// Comparison
String* other = string_from_cstr("hello, beautiful World!");
if (string_equals(str, other)) {
    printf("Strings are equal\n");
}

// Cleanup
string_destroy(str);
string_destroy(other);
```

### 9. Trie (Prefix Tree)
A header-only trie implementation for efficient string storage and prefix matching.

**Key Features:**
- 29-character alphabet (a-z, apostrophe, hyphen, period)
- Word normalization (lowercase, digit removal, smart apostrophe handling)
- Space-efficient prefix sharing
- Fast prefix-based queries

**Usage:**
```c
// Create trie
Trie* trie = trie_create();

// Insert words
trie_insert_cstr(trie, "hello");
trie_insert_cstr(trie, "world");
trie_insert_cstr(trie, "help");

// Cleanup
trie_destroy(trie);
```

## Example Application: Word Frequency Counter

The library includes a Shakespeare text parser that demonstrates practical usage:

```c
int parse(void) {
    // Create hashmap with String keys and int values
    hashmap* map = hashmap_create(
        sizeof(String), 
        sizeof(int),
        murmurhash3_string,
        string_custom_delete,
        NULL,
        string_custom_compare
    );

    // Process file and count word frequencies
    FILE* f = fopen("shakespeare.txt", "r");
    char line[512];
    char cleaned[256];
    
    while (fgets(line, sizeof(line), f)) {
        char* token = strtok(line, " \n\t\r");
        while (token) {
            // Clean word (remove numbers, handle contractions)
            if (clean_word(token, cleaned, sizeof(cleaned))) {
                String str;
                string_create_onstack(&str, cleaned);
                
                int count;
                if (hashmap_get(map, (u8*)&str, (u8*)&count) == 0) {
                    count++;
                } else {
                    count = 1;
                }
                hashmap_put(map, (u8*)&str, (u8*)&count);
            }
            token = strtok(NULL, " \n\t\r");
        }
    }
    
    // Query results
    String query;
    string_create_onstack(&query, "romeo");
    int count;
    if (hashmap_get(map, (u8*)&query, (u8*)&count) == 0) {
        printf("Count of %s: %d\n", string_to_cstr(&query), count);
    }
    
    // Cleanup
    string_destroy_fromstk(&query);
    hashmap_destroy(map);
    fclose(f);
    return 0;
}
```

## Helper Functions

The library includes convenient wrapper functions in `helper_functions.h`:

```c
// Vector helpers
vec_push_int(vec, 42);
int val = vec_get_int(vec, 0);

// HashMap helpers
map_put_strToInt(map, "key", 100);
int value = map_get_strToInt(map, "key");

// HashSet helpers
set_insert_int(set, 42);
if (set_has_int(set, 42)) { /* ... */ }

// Print functions
int_print((u8*)&value);
str_print((u8*)&string);

// Comparison functions
u8 cmp = int_cmp((u8*)&a, (u8*)&b);  // Returns 1, 0, or 255

// Conversion functions
String* str = int_to_str((u8*)&value);
```

## Design Principles

### Memory Management
- **Ownership**: Data structures take ownership of inserted data
- **Custom Deleters**: Optional delete functions for cleanup of complex types
- **RAII-style**: Destroy functions handle all cleanup automatically
- **Stack Support**: Many structures support stack allocation with separate cleanup

### Type Safety
- Uses `uint8_t*` (`u8*`) for generic byte manipulation
- Size parameters ensure type safety at runtime
- Function pointers for type-specific operations

### Performance
- Cache-friendly memory layout with alignment attributes
- Prime number capacities for hash tables
- Dynamic resizing with configurable load factors
- Linear probing for cache locality
- Circular buffer for queue efficiency

## Building

```bash
gcc -O2 -Wall -Wextra \
    main.c \
    gen_vector.c \
    hashmap.c \
    hashset.c \
    String.c \
    BST.c \
    bit_vector.c \
    Queue.c \
    Stack.c \
    -o program
```

## API Reference

### Vector Operations
- `genVec_init()` - Create vector
- `genVec_init_stk()` - Initialize on stack
- `genVec_push()` - Add element to end
- `genVec_pop()` - Remove last element
- `genVec_get()` / `genVec_get_ptr()` - Access element
- `genVec_insert()` / `genVec_insert_multi()` - Insert operations
- `genVec_remove()` - Remove at position
- `genVec_replace()` - Replace element
- `genVec_reserve()` / `genVec_reserve_val()` - Pre-allocate capacity
- `genVec_copy()` - Shallow copy
- `genVec_destroy()` / `genVec_destroy_stk()` - Cleanup

### HashMap Operations
- `hashmap_create()` - Create hashmap
- `hashmap_put()` - Insert/update key-value pair
- `hashmap_get()` - Retrieve value by key
- `hashmap_del()` - Remove key-value pair
- `hashmap_has()` - Check key existence
- `hashmap_modify()` - Modify value in-place with callback
- `hashmap_print()` - Print all key-value pairs
- `hashmap_destroy()` - Free all memory

### HashSet Operations
- `hashset_create()` - Create hashset
- `hashset_insert()` - Add element (returns 0 if already exists)
- `hashset_has()` - Check element existence
- `hashset_remove()` - Remove element
- `hashset_print()` - Print all elements
- `hashset_destroy()` - Free all memory

### BST Operations
- `bst_create()` - Create BST
- `bst_insert()` - Insert element
- `bst_remove()` - Remove element
- `bst_search()` - Search for element
- `bst_find_min()` / `bst_find_max()` - Find extrema
- `bst_preorder()` / `bst_inorder()` / `bst_postorder()` - DFS traversals
- `bst_bfs()` - Level-order traversal
- `bst_balance()` - Balance tree (experimental)
- `bst_destroy()` - Free all memory

### Bit Vector Operations
- `bitVec_create()` - Create bit vector
- `bitVec_set()` - Set bit to 1
- `bitVec_clear()` - Set bit to 0
- `bitVec_test()` - Test if bit is set
- `bitVec_toggle()` - Flip bit value
- `bitVec_push()` - Add set bit
- `bitVec_pop()` - Remove last bit
- `bitVec_print()` - Print byte contents
- `bitVec_destroy()` - Free memory

### Queue Operations
- `queue_create()` - Create queue
- `enqueue()` - Add element to back
- `dequeue()` - Remove element from front
- `queue_peek()` - View front element
- `queue_size()` / `queue_empty()` - Query state
- `queue_print()` - Print contents
- `queue_destroy()` - Free memory

### Stack Operations
- `stack_create()` - Create stack
- `stack_push()` - Push element
- `stack_pop()` - Pop element
- `stack_peek()` - View top element
- `stack_size()` - Get size
- `stack_print()` - Print contents
- `stack_destroy()` - Free memory

### String Operations
- `string_create()` / `string_from_cstr()` - Create string
- `string_create_onstack()` - Initialize on stack
- `string_append_*()` - Append operations
- `string_insert_*()` - Insert operations
- `string_remove_char()` - Remove character
- `string_compare()` / `string_equals()` - Comparison
- `string_find_*()` - Search operations
- `string_substr()` - Extract substring
- `string_at()` / `string_set_char()` - Character access
- `string_len()` / `string_empty()` - Query state
- `string_destroy()` / `string_destroy_fromstk()` - Cleanup

## Configuration

Key constants in `default_functions.h`:
```c
#define LOAD_FACTOR_GROW 0.70    // Resize up threshold
#define LOAD_FACTOR_SHRINK 0.20  // Resize down threshold
#define HASHMAP_INIT_CAPACITY 17 // Initial capacity (prime)
```

Key constants for growth/shrink in `gen_vector.c`:
```c
#define GROWTH 1.5              // Growth multiplier
#define SHRINK_AT 0.25          // Shrink when 25% full
#define SHRINK_BY 0.5           // Shrink to 50% capacity
```

## Internal Structure

### HashMap/HashSet States
```c
typedef enum {
    EMPTY = 0,      // Bucket never used
    FILLED = 1,     // Bucket contains valid data
    TOMBSTONE = 2   // Bucket previously used, now deleted
} STATE;
```

### BST Array Indexing
```c
#define PARENT(i)  (((i) - 1) / 2)
#define L_CHILD(i) ((2 * (i)) + 1)
#define R_CHILD(i) ((2 * (i)) + 2)
```

### Word Cleaning Features
The parser includes advanced word normalization:
- Removes all digits
- Handles contractions (keeps apostrophes in words like "don't")
- Removes possessive forms (e.g., "Shakespeare's" → "shakespeare")
- Handles UTF-8 curly quotes
- Case-insensitive processing

## Known Issues

- **BST Balance**: The `bst_balance()` function is marked as experimental and may have correctness issues
- **Queue Shrinking**: Queue does not shrink to optimize for performance
- **String Helper Functions**: Some string vector helpers in `helper_functions.h` have memory management issues (see WARN/TODO comments)

