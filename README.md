# Generic Data Structures Library in C

A collection of high-performance, generic data structures implemented in C, including vectors, hashmaps, hashsets, and dynamic strings.

## Features

- **Generic Vector (`genVec`)**: Dynamic array with automatic resizing
- **HashMap**: Hash table with linear probing and dynamic resizing
- **HashSet**: Set implementation using hash table
- **String**: Dynamic string with rich manipulation API
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

int value2 = 100;
hashset_insert(set, (u8*)&value2);

// Check membership
if (hashset_has(set, (u8*)&value)) {
    printf("Element 42 found!\n");
}

// Remove element
hashset_remove(set, (u8*)&value);

// Cleanup
hashset_destroy(set);
```

**Advanced Example with Strings:**
```c
// Create a hashset of unique words
hashset* word_set = hashset_create(
    sizeof(String),
    murmurhash3_string,   // Custom hash for String
    string_custom_delete, // Delete function for String
    string_custom_compare // Compare function for String
);

// Insert words
String word1, word2;
string_create_onstack(&word1, "hello");
string_create_onstack(&word2, "world");

hashset_insert(word_set, (u8*)&word1);
hashset_insert(word_set, (u8*)&word2);

// Check if word exists
String query;
string_create_onstack(&query, "hello");
if (hashset_has(word_set, (u8*)&query)) {
    printf("Word exists in set!\n");
}

// Print all elements
void print_string_element(const u8* elm) {
    String* str = (String*)elm;
    printf("\"%s\"", string_to_cstr(str));
}
hashset_print(word_set, print_string_element);

// Cleanup
string_destroy_fromstk(&word1);
string_destroy_fromstk(&word2);
string_destroy_fromstk(&query);
hashset_destroy(word_set);
```

### 4. Dynamic String
A robust string implementation with automatic memory management.

**Key Features:**
- Built on top of `genVec`
- Automatic null termination
- Rich string manipulation API
- Support for both heap and stack allocation

**Usage:**
```c
// Create from C string
String* str = string_from_cstr("Hello");

// Append operations
string_append_cstr(str, " World");
string_append_char(str, '!');

// Access and modify
char c = string_at(str, 0);
string_set_char(str, 0, 'h');

// Comparison
String* other = string_from_cstr("hello World!");
if (string_equals(str, other)) {
    printf("Strings are equal\n");
}

// Cleanup
string_destroy(str);
string_destroy(other);
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

## Design Principles

### Memory Management
- **Ownership**: Data structures take ownership of inserted data
- **Custom Deleters**: Optional delete functions for cleanup of complex types
- **RAII-style**: Destroy functions handle all cleanup automatically

### Type Safety
- Uses `uint8_t*` (`u8*`) for generic byte manipulation
- Size parameters ensure type safety at runtime
- Function pointers for type-specific operations

### Performance
- Cache-friendly memory layout with alignment attributes
- Prime number capacities for hash tables (HashMap only)
- Dynamic resizing with configurable load factors
- Linear probing for cache locality

## Building

```bash
gcc -O2 -Wall -Wextra \
    main.c \
    gen_vector.c \
    hashmap.c \
    hashset.c \
    String.c \
    -o program
```

## API Reference

### Vector Operations
- `genVec_init()` - Create vector
- `genVec_push()` - Add element to end
- `genVec_pop()` - Remove last element
- `genVec_get()` - Access element by index
- `genVec_insert()` - Insert at position
- `genVec_insert_multi()` - Insert multiple elements
- `genVec_remove()` - Remove at position
- `genVec_replace()` - Replace element at position
- `genVec_front()` / `genVec_back()` - Access first/last element
- `genVec_copy()` - Create shallow copy
- `genVec_destroy()` - Free all memory

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

### String Operations
- `string_create()` / `string_from_cstr()` - Create string
- `string_create_onstack()` - Initialize string on stack
- `string_append_*()` - Append operations
- `string_insert_*()` - Insert operations
- `string_remove_char()` - Remove character at position
- `string_compare()` / `string_equals()` - Comparison
- `string_find_*()` - Search operations
- `string_substr()` - Extract substring
- `string_at()` / `string_set_char()` - Character access/modification
- `string_destroy()` / `string_destroy_fromstk()` - Free memory

## Configuration

Key constants in `default_functions.h`:
```c
#define LOAD_FACTOR_GROW 0.70    // Resize up threshold
#define LOAD_FACTOR_SHRINK 0.20  // Resize down threshold
#define HASHMAP_INIT_CAPACITY 17 // Initial capacity (prime)
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

### Word Cleaning Features
The parser includes advanced word normalization:
- Removes all digits
- Handles contractions (keeps apostrophes in words like "don't")
- Removes possessive forms (e.g., "Shakespeare's" → "shakespeare")
- Handles UTF-8 curly quotes
- Case-insensitive processing

## License

This is a personal educational project. Feel free to use and modify as needed.

