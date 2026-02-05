#ifndef HASHMAP_SINGLE_H
#define HASHMAP_SINGLE_H



#ifndef GENVEC_SINGLE_H
#define GENVEC_SINGLE_H

#ifndef COMMON_H
#define COMMON_H


// LOGGING/ERRORS

#include <stdio.h>
#include <stdlib.h>

#define WARN(fmt, ...)                                                                       \
    do {                                                                                     \
        printf("[WARN] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

#define FATAL(fmt, ...)                                                                \
    do {                                                                               \
        fprintf(stderr, "[FATAL] %s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __func__, \
                ##__VA_ARGS__);                                                        \
        exit(EXIT_FAILURE);                                                            \
    } while (0)


#define ASSERT_WARN(cond, fmt, ...)                                                  \
    do {                                                                             \
        if (!(cond)) { WARN("Assertion failed: (%s): " fmt, #cond, ##__VA_ARGS__); } \
    } while (0)

#define ASSERT_WARN_RET(cond, ret, fmt, ...)                            \
    do {                                                                \
        if (!(cond)) {                                                  \
            WARN("Assertion failed: (%s): " fmt, #cond, ##__VA_ARGS__); \
            return ret;                                                 \
        }                                                               \
    } while (0)

#define ASSERT_FATAL(cond, fmt, ...)                                                  \
    do {                                                                              \
        if (!(cond)) { FATAL("Assertion failed: (%s): " fmt, #cond, ##__VA_ARGS__); } \
    } while (0)

#define CHECK_WARN(cond, fmt, ...)                                       \
    do {                                                                 \
        if ((cond)) { WARN("Check: (%s): " fmt, #cond, ##__VA_ARGS__); } \
    } while (0)


#define CHECK_WARN_RET(cond, ret, fmt, ...)                  \
    do {                                                     \
        if ((cond)) {                                        \
            WARN("Check: (%s): " fmt, #cond, ##__VA_ARGS__); \
            return ret;                                      \
        }                                                    \
    } while (0)

#define CHECK_FATAL(cond, fmt, ...)                                     \
    do {                                                                \
        if (cond) { FATAL("Check: (%s): " fmt, #cond, ##__VA_ARGS__); } \
    } while (0)


// TYPES

#include <stdint.h>

typedef uint8_t  u8;
typedef uint8_t  b8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define false ((b8)0)
#define true  ((b8)1)


// CASTING

#define cast(x)    ((u8*)(&(x)))
#define castptr(x) ((u8*)(x))


// COMMON SIZES

#define KB (1 << 10)
#define MB (1 << 20)

#define nKB(n) ((u32)((n) * KB))
#define nMB(n) ((u32)((n) * MB))


// RAW BYTES TO HEX

void print_hex(const u8* ptr, u32 size, u32 bytes_per_line) 
{
    if (ptr == NULL | size == 0 | bytes_per_line == 0) { return; }

    // hex rep 0-15
    const char* hex = "0123456789ABCDEF";
    
    for (u32 i = 0; i < size; i++) 
    {
        u8 val1 = ptr[i] >> 4;      // get upper 4 bits as num b/w 0-15
        u8 val2 = ptr[i] & 0x0F;    // get lower 4 bits as num b/w 0-15
        
        printf("%c%c", hex[val1], hex[val2]);
        
        // Add space or newline appropriately
        if ((i + 1) % bytes_per_line == 0) {
            printf("\n");
        } else if (i < size - 1) {
            printf(" ");
        }
    }

    // Add final newline if we didn't just print one
    if (size % bytes_per_line != 0) {
        printf("\n");
    }
}


#endif // COMMON_H


/*          TLDR
 * genVec is a value-based generic vector.
 * Elements are stored inline and managed via user-supplied
 * copy/move/destructor callbacks.
 *
 * This avoids pointer ownership ambiguity and improves cache locality.
 */


// User-provided callback functions
typedef void (*genVec_print_fn)(const u8* elm);
typedef b8 (*genVec_compare_fn)(const u8* a, const u8* b);
typedef void (*genVec_delete_fn)(u8* elm);               // Cleanup owned resources
typedef void (*genVec_copy_fn)(u8* dest, const u8* src); // Deep copy resources
typedef void (*genVec_move_fn)(u8* dest, u8** src);      // Move src into dest, null src


// genVec growth/shrink settings (user can change)
#ifndef GENVEC_GROWTH
    #define GENVEC_GROWTH 1.5F // vec capacity multiplier
#endif
#ifndef GENVEC_SHRINK_AT
    #define GENVEC_SHRINK_AT 0.25F // % filled to shrink at (25% filled)
#endif
#ifndef GENVEC_SHRINK_BY
    #define GENVEC_SHRINK_BY 0.5F // capacity dividor (half)
#endif


//      SMALL VECTOR OPTIMIZATION
// if vector data takes <= 64 bytes, store it on the stack in the struct itself using union
// entire union takes 64 bytes in the struct
// when we heap alloc vector, then this is also on heap (not much improvement there)
// but with stack vector, whole vector is then on the stack
#ifndef GENVEC_SVO_SIZE
    #define GENVEC_SVO_SIZE 64
#endif



// generic vector container
typedef struct {
    // Contiguous array of elements
    union {
        u8  stack[GENVEC_SVO_SIZE]; // for small vectors (on stack) -> capacity <= SVO_CAPACITY
        u8* heap;                   // for large vectors (on heap)  -> capacity > SVO_CAPACITY
    } data;

    u32 size;      // Number of elements currently in vector
    u32 capacity;  // Total allocated capacity
    u16 data_size; // Size of each element in bytes
    b8  svo;       // Flag to determine if data is on stack or heap

    genVec_copy_fn   copy_fn; // Deep copy function for owned resources (or NULL)
    genVec_move_fn   move_fn; // Get a double pointer, transfer ownership and null original
    genVec_delete_fn del_fn;  // Cleanup function for owned resources (or NULL)
} genVec;


// Memory Management
// ===========================

// Initialize vector with capacity n. If elements own heap resources,
// provide copy_fn (deep copy) and del_fn (cleanup). Otherwise pass NULL.
genVec* genVec_init(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn);

// Initialize vector on stack with data on heap
// SVO works best here as it is on the stack***
void genVec_init_stk(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn,
                     genVec* vec);

// Initialize vector of size n, all elements set to val
genVec* genVec_init_val(u32 n, const u8* val, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn,
                        genVec_delete_fn del_fn);

void genVec_init_val_stk(u32 n, const u8* val, u16 data_size, 
        genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn, genVec* vec);

// Destroy heap-allocated vector and clean up all elements
void genVec_destroy(genVec* vec);

// Destroy stack-allocated vector (cleans up data, but not vec itself)
void genVec_destroy_stk(genVec* vec);

// Remove all elements (calls del_fn on each), keep capacity
void genVec_clear(genVec* vec);

// Remove all elements and free memory, shrink capacity to 0
void genVec_reset(genVec* vec);

// Ensure vector has at least new_capacity space (never shrinks)
void genVec_reserve(genVec* vec, u32 new_capacity);

// Grow to new_capacity and fill new slots with val
void genVec_reserve_val(genVec* vec, u32 new_capacity, const u8* val);


// Operations
// ===========================

// Append element to end (makes deep copy if copy_fn provided)
void genVec_push(genVec* vec, const u8* data);

// Append element to end, transfer ownership (nulls original pointer)
void genVec_push_move(genVec* vec, u8** data);

// Remove element from end. If popped provided, copies element before deletion.
// Note: del_fn is called regardless to clean up owned resources.
void genVec_pop(genVec* vec, u8* popped);

// Copy element at index i into out buffer
void genVec_get(const genVec* vec, u32 i, u8* out);

// Get pointer to element at index i
// Note: Pointer invalidated by push/insert/remove operations
const u8* genVec_get_ptr(const genVec* vec, u32 i);

// apply a function on each value of the array
void genVec_for_each(genVec* vec, void (*for_each)(u8* elm));

// Replace element at index i with data (cleans up old element)
void genVec_replace(genVec* vec, u32 i, const u8* data);

// Replace element at index i, transfer ownership (cleans up old element)
void genVec_replace_move(genVec* vec, u32 i, u8** data);

// Insert element at index i, shifting elements right
void genVec_insert(genVec* vec, u32 i, const u8* data);

// Insert element at index i with ownership transfer, shifting elements right
void genVec_insert_move(genVec* vec, u32 i, u8** data);

// Insert num_data elements from data arr to vec. data should have same size data as vec
void genVec_insert_multi(genVec* vec, u32 i, const u8* data, u32 num_data);

// Insert (move) num_data  elements from data starting at index i. Transfers ownership
void genVec_insert_multi_move(genVec* vec, u32 i, u8** data, u32 num_data);

// Remove element at index i, optionally copy to out, shift elements left
void genVec_remove(genVec* vec, u32 i, u8* out);

// Remove elements in range [l, r] inclusive.
void genVec_remove_range(genVec* vec, u32 l, u32 r);

// Get pointer to first element
const u8* genVec_front(const genVec* vec);

// Get pointer to last element
const u8* genVec_back(const genVec* vec);


// Utility
// ===========================

// Print all elements using provided print function
void genVec_print(const genVec* vec, genVec_print_fn fn);

// Deep copy src vector into dest
// Note: cleans up dest (if already inited)
void genVec_copy(genVec* dest, const genVec* src);

// transfers ownership from src to dest
// Note: src must be heap-allocated
void genVec_move(genVec* dest, genVec** src);


// Get number of elements in vector
static inline u32 genVec_size(const genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    return vec->size;
}

// Get total capacity of vector
static inline u32 genVec_capacity(const genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    return vec->capacity;
}

// Check if vector is empty
static inline u8 genVec_empty(const genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    return vec->size == 0;
}


// Get maximum number of elements that can fit in SVO storage
#define GENVEC_SVO_CAPACITY(data_size) (GENVEC_SVO_SIZE / (data_size))

// Check if vector is using stack storage (SVO)
static inline b8 genVec_isSVO(const genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    return vec->svo;
}



#include <stdlib.h>
#include <string.h>


#define GENVEC_MIN_CAPACITY 4


// Helper macros

// Get pointer to data array (handles both stack and heap storage)
#define GET_DATA(vec) ((vec)->svo ? (vec)->data.stack : (vec)->data.heap)
// get ptr to elm at index i
#define GET_PTR(vec, i) (GET_DATA((vec)) + ((u64)(i) * ((vec)->data_size)))
// get total_size in bytes for i elements
#define GET_SCALED(vec, i) ((u64)(i) * ((vec)->data_size))
// check if we need to grow vector
#define MAYBE_GROW(vec)                                                                   \
    do {                                                                                  \
        if (!vec->svo && !vec->data.heap) {                                               \
            genVec_grow(vec);                                                             \
        } else if (vec->size >= vec->capacity) {                                          \
            if (vec->svo) {                                                               \
                genVec_migrate_to_heap(vec, (u32)((float)vec->capacity * GENVEC_GROWTH)); \
            } else {                                                                      \
                genVec_grow(vec);                                                         \
            }                                                                             \
        }                                                                                 \
    } while (0)
// check if we need to shrink vector
#define MAYBE_SHRINK(vec)                                                               \
    do {                                                                                \
        if (!vec->svo && vec->size <= (u32)((float)vec->capacity * GENVEC_SHRINK_AT)) { \
            genVec_shrink(vec);                                                         \
        }                                                                               \
    } while (0)


//private functions

void genVec_grow(genVec* vec);
void genVec_shrink(genVec* vec);
void genVec_migrate_to_heap(genVec* vec, u32 new_capacity);


// API Implementation

genVec* genVec_init(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn)
{
    CHECK_FATAL(data_size == 0, "data_size can't be 0");

    genVec* vec = (genVec*)malloc(sizeof(genVec));
    CHECK_FATAL(!vec, "vec init failed");

    // Calculate how many elements can fit in SVO storage
    u32 svo_cap = GENVEC_SVO_CAPACITY(data_size);

    // Use SVO if requested capacity fits
    if (/*n > 0 &&*/ n <= svo_cap) {
        vec->svo      = true;
        vec->capacity = svo_cap; // in stack mode if size <= svo_cap
    } else {
        // Only allocate memory if n > 0, otherwise data can be NULL
        vec->data.heap = (n > 0) ? (u8*)malloc((size_t)data_size * n) : NULL;

        // Only check for allocation failure if we actually tried to allocate
        if (n > 0 && !vec->data.heap) {
            free(vec);
            FATAL("data init failed");
        }

        vec->svo      = false;
        vec->capacity = n;
    }

    vec->size      = 0;
    vec->data_size = data_size;

    vec->copy_fn = copy_fn;
    vec->move_fn = move_fn;
    vec->del_fn  = del_fn;

    return vec;
}


void genVec_init_stk(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn,
                     genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(data_size == 0, "data_size can't be 0");


    u32 svo_cap = GENVEC_SVO_CAPACITY(data_size);

    if (n <= svo_cap) {
        vec->svo      = true;
        vec->capacity = svo_cap;
    } else {
        // Only allocate memory if n > 0, otherwise data can be NULL
        vec->data.heap = (n > 0) ? (u8*)malloc((size_t)data_size * n) : NULL;
        CHECK_FATAL(n > 0 && !vec->data.heap, "data init failed");

        vec->svo      = false;
        vec->capacity = n;
    }

    vec->size      = 0;
    vec->data_size = data_size;
    vec->copy_fn   = copy_fn;
    vec->move_fn   = move_fn;
    vec->del_fn    = del_fn;
}

genVec* genVec_init_val(u32 n, const u8* val, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn,
                        genVec_delete_fn del_fn)
{
    CHECK_FATAL(!val, "val can't be null");
    CHECK_FATAL(n == 0, "cant init with val if n = 0");

    genVec* vec = genVec_init(n, data_size, copy_fn, move_fn, del_fn);

    vec->size = n; //capacity set to n in upper func

    for (u32 i = 0; i < n; i++) {
        if (copy_fn) {
            copy_fn(GET_PTR(vec, i), val);
        } else {
            memcpy(GET_PTR(vec, i), val, data_size);
        }
    }

    return vec;
}

void genVec_init_val_stk(u32 n, const u8* val, u16 data_size, 
        genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn, genVec* vec)
{
    CHECK_FATAL(!val, "val can't be null");
    CHECK_FATAL(n == 0, "cant init with val if n = 0");

    genVec_init_stk(n, data_size, copy_fn, move_fn, del_fn, vec);

    vec->size = n;

    for (u32 i = 0; i < n; i++) {
        if (copy_fn) {
            copy_fn(GET_PTR(vec, i), val);
        } else {
            memcpy(GET_PTR(vec, i), val, data_size);
        }
    }
}

void genVec_destroy(genVec* vec)
{
    genVec_destroy_stk(vec);

    free(vec);
}


void genVec_destroy_stk(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (!vec->svo && !vec->data.heap) {
        return;
    }

    if (vec->del_fn) {
        // Custom cleanup for each element
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }

    // Free heap data if in heap mode
    if (!vec->svo && vec->data.heap) {
        free(vec->data.heap);
        vec->data.heap = NULL;
    }
    // dont free vec as on stk (don't own memory)
}

void genVec_clear(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (vec->del_fn) { // owns resources
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }
    // doesn't free container
    vec->size = 0;
}

void genVec_reset(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (vec->del_fn) {
        for (u32 i = 0; i < vec->size; i++) {
            vec->del_fn(GET_PTR(vec, i));
        }
    }

    if (!vec->svo) {
        free(vec->data.heap);
        vec->data.heap = NULL;
        vec->capacity  = GENVEC_SVO_CAPACITY(vec->data_size);
        vec->svo       = true; // only time it's reset
    }

    vec->size = 0;
}


void genVec_reserve(genVec* vec, u32 new_capacity)
{
    CHECK_FATAL(!vec, "vec is null");

    // Only grow, never shrink with reserve
    if (new_capacity <= vec->capacity) {
        return;
    }

    if (vec->svo) {
        genVec_migrate_to_heap(vec, new_capacity); // cap is greater than curr cap
        return;
    }

    u8* new_data = realloc(vec->data.heap, GET_SCALED(vec, new_capacity));
    CHECK_FATAL(!new_data, "realloc failed");

    vec->data.heap = new_data;
    vec->capacity  = new_capacity;
}

void genVec_reserve_val(genVec* vec, u32 new_capacity, const u8* val)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!val, "val is null");
    CHECK_FATAL(new_capacity < vec->size, "new_capacity must be >= current size");

    genVec_reserve(vec, new_capacity);

    for (u32 i = vec->size; i < new_capacity; i++) {
        if (vec->copy_fn) {
            vec->copy_fn(GET_PTR(vec, i), val);
        } else {
            memcpy(GET_PTR(vec, i), val, vec->data_size);
        }
    }
    vec->size = new_capacity;
}


void genVec_push(genVec* vec, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");

    // Check if we need to allocate or grow
    MAYBE_GROW(vec);

    if (vec->copy_fn) {
        vec->copy_fn(GET_PTR(vec, vec->size), data);
    } else {
        memcpy(GET_PTR(vec, vec->size), data, vec->data_size);
    }

    vec->size++;
}


void genVec_push_move(genVec* vec, u8** data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(!*data, "*data is null");

    // Check if we need to allocate or grow
    MAYBE_GROW(vec);

    if (vec->move_fn) {
        vec->move_fn(GET_PTR(vec, vec->size), data);
    } else {
        // copy the pointer to resource
        memcpy(GET_PTR(vec, vec->size), *data, vec->data_size);
        *data = NULL; // now arr owns the resource
    }

    vec->size++;
}


// pop can't be a move operation (array is contiguos)
void genVec_pop(genVec* vec, u8* popped)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_WARN_RET(vec->size == 0, , "vec is empty");

    u8* last_elm = GET_PTR(vec, vec->size - 1);

    if (popped) {
        if (vec->copy_fn) {
            vec->copy_fn(popped, last_elm);
        } else {
            memcpy(popped, last_elm, vec->data_size);
        }
    }

    if (vec->del_fn) { // del func frees the resources owned by last_elm, but not ptr
        vec->del_fn(last_elm);
    }

    vec->size--; // set for re-write

    MAYBE_SHRINK(vec);
}


void genVec_get(const genVec* vec, u32 i, u8* out)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");
    CHECK_FATAL(!out, "need a valid out variable to get the element");

    if (vec->copy_fn) {
        vec->copy_fn(out, GET_PTR(vec, i));
    } else {
        memcpy(out, GET_PTR(vec, i), vec->data_size);
    }
}


const u8* genVec_get_ptr(const genVec* vec, u32 i)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");

    return GET_PTR(vec, i);
}


void genVec_for_each(genVec* vec, void (*for_each)(u8* elm))
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!for_each, "for_each function is null");

    for (u32 i = 0; i < vec->size; i++) {
        for_each(GET_PTR(vec, i));
    }
}


void genVec_insert(genVec* vec, u32 i, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(i > vec->size, "index out of bounds");

    if (i == vec->size) {
        genVec_push(vec, data);
        return;
    }

    MAYBE_GROW(vec);

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;
    // the place where we want to insert
    u8* src = GET_PTR(vec, i);

    // Shift elements right by one unit
    u8* dest = GET_PTR(vec, i + 1);
    memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // Use memmove for overlapping regions

    //src pos is now free to insert (it's data copied to next location)
    if (vec->copy_fn) {
        vec->copy_fn(src, data);
    } else {
        memcpy(src, data, vec->data_size);
    }

    vec->size++;
}

void genVec_insert_move(genVec* vec, u32 i, u8** data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(!*data, "*data is null");
    CHECK_FATAL(i > vec->size, "index out of bounds");

    if (i == vec->size) {
        genVec_push_move(vec, data);
        return;
    }

    MAYBE_GROW(vec);

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;
    // the place where we want to insert
    u8* src = GET_PTR(vec, i);

    // Shift elements right by one unit
    u8* dest = GET_PTR(vec, i + 1);
    memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // Use memmove for overlapping regions


    if (vec->move_fn) {
        vec->move_fn(src, data);
    } else {
        memcpy(src, *data, vec->data_size);
        *data = NULL;
    }

    vec->size++;
}


void genVec_insert_multi(genVec* vec, u32 i, const u8* data, u32 num_data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(num_data == 0, "num_data can't be 0");
    CHECK_FATAL(i > vec->size, "index out of bounds");

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;

    vec->size += num_data; // no of new elements in chunk

    genVec_reserve(vec, vec->size); // reserve with new size

    // the place where we want to insert
    u8* src = GET_PTR(vec, i);
    if (elements_to_shift > 0) {
        // Shift elements right by num_data units to right
        u8* dest = GET_PTR(vec, i + num_data);

        memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // using memmove for overlapping regions
    }

    //src pos is now free to insert (it's data copied to next location)
    if (vec->copy_fn) {
        for (u32 j = 0; j < num_data; j++) {
            vec->copy_fn(GET_PTR(vec, j + i), (data + (size_t)(j * vec->data_size)));
        }
    } else {
        memcpy(src, data, GET_SCALED(vec, num_data));
    }
}

void genVec_insert_multi_move(genVec* vec, u32 i, u8** data, u32 num_data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(!*data, "*data is null");
    CHECK_FATAL(num_data == 0, "num_data can't be 0");
    CHECK_FATAL(i > vec->size, "index out of bounds");

    // Calculate the number of elements to shift to right
    u32 elements_to_shift = vec->size - i;

    vec->size += num_data; // no of new elements in chunk

    genVec_reserve(vec, vec->size); // reserve with new size

    // the place where we want to insert
    u8* src = GET_PTR(vec, i);
    if (elements_to_shift > 0) {
        // Shift elements right by num_data units to right
        u8* dest = GET_PTR(vec, i + num_data);

        memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // using memmove for overlapping regions
    }

    //src pos is now free to insert (it's data copied to next location)
    // Move entire contiguous block at once
    memcpy(src, *data, GET_SCALED(vec, num_data));
    *data = NULL; // Transfer ownership
}


void genVec_remove(genVec* vec, u32 i, u8* out)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");

    if (out) {
        if (vec->copy_fn) {
            vec->copy_fn(out, GET_PTR(vec, i));
        } else {
            memcpy(out, GET_PTR(vec, i), vec->data_size);
        }
    }

    if (vec->del_fn) {
        vec->del_fn(GET_PTR(vec, i));
    }
    // Calculate the number of elements to shift
    u32 elements_to_shift = vec->size - i - 1;

    if (elements_to_shift > 0) {
        // Shift elements left to overwrite the deleted element
        u8* dest = GET_PTR(vec, i);
        u8* src  = GET_PTR(vec, i + 1);

        memmove(dest, src, GET_SCALED(vec, elements_to_shift)); // Use memmove for overlapping regions
    }

    vec->size--;

    MAYBE_SHRINK(vec);
}


void genVec_remove_range(genVec* vec, u32 l, u32 r)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(l >= vec->size, "index out of range");
    CHECK_FATAL(l > r, "invalid range");

    if (r >= vec->size) {
        r = vec->size - 1;
    }

    if (vec->del_fn) {
        for (u32 i = l; i <= r; i++) {
            u8* elm = GET_PTR(vec, i);
            vec->del_fn(elm);
        }
    }

    u32 elms_to_shift = vec->size - (r + 1);

    // move from r + 1 to l
    u8* dest = GET_PTR(vec, l);
    u8* src  = GET_PTR(vec, r + 1);
    memmove(dest, src, GET_SCALED(vec, elms_to_shift)); // Use memmove for overlapping regions

    vec->size -= (r - l + 1);

    MAYBE_SHRINK(vec);
}


void genVec_replace(genVec* vec, u32 i, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");
    CHECK_FATAL(!data, "need a valid data variable");

    u8* to_replace = GET_PTR(vec, i);

    if (vec->del_fn) {
        vec->del_fn(to_replace);
    }

    if (vec->copy_fn) {
        vec->copy_fn(to_replace, data);
    } else {
        memcpy(to_replace, data, vec->data_size);
    }
}


void genVec_replace_move(genVec* vec, u32 i, u8** data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");
    CHECK_FATAL(!data, "need a valid data variable");
    CHECK_FATAL(!*data, "need a valid *data variable");

    u8* to_replace = GET_PTR(vec, i);

    if (vec->del_fn) {
        vec->del_fn(to_replace);
    }

    if (vec->move_fn) {
        vec->move_fn(to_replace, data);
    } else {
        memcpy(to_replace, *data, vec->data_size);
        *data = NULL;
    }
}


const u8* genVec_front(const genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(vec->size == 0, "vec is empty");

    return GET_DATA(vec);
}


const u8* genVec_back(const genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(vec->size == 0, "vec is empty");

    return GET_PTR(vec, vec->size - 1);
}


void genVec_print(const genVec* vec, genVec_print_fn fn)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!fn, "print func is null");

    printf("[ ");
    for (u32 i = 0; i < vec->size; i++) {
        fn(GET_PTR(vec, i));
        printf(" ");
    }
    printf("]\n");

    printf("Size: %u\n", vec->size);
    printf("Capacity: %u\n", vec->capacity);
}


void genVec_copy(genVec* dest, const genVec* src)
{
    CHECK_FATAL(!dest, "dest is null");
    CHECK_FATAL(!src, "src is null");

    if (dest == src) {
        return;
    }

    // if data ptr is null, no op
    genVec_destroy_stk(dest);

    // copy all fields
    memcpy(dest, src, sizeof(genVec));

    if (src->svo) { // we already have all elements
        return;
    }

    // malloc data ptr
    dest->data.heap = (u8*)malloc(GET_SCALED(src, src->capacity));

    // Copy elements
    if (src->copy_fn) {
        for (u32 i = 0; i < src->size; i++) {
            src->copy_fn(GET_PTR(dest, i), GET_PTR(src, i));
        }
    } else {
        memcpy(dest->data.heap, src->data.heap, GET_SCALED(src, src->size));
    }
}


void genVec_move(genVec* dest, genVec** src)
{
    CHECK_FATAL(!src, "src is null");
    CHECK_FATAL(!*src, "*src is null");
    CHECK_FATAL(!dest, "dest is null");
    // CHECK_FATAL((*src)->svo, "can't move a SVO (stack) vec");

    if (dest == *src) {
        *src = NULL;
        return;
    }

    // Transfer all fields from src to dest
    memcpy(dest, *src, sizeof(genVec));

    if ((*src)->svo) {
        (*src)->size = 0; // can't acces data anymore
        return;
    }

    // Null out src's data pointer so it doesn't get freed
    (*src)->data.heap = NULL;

    // Free src if it was heap-allocated
    // This only frees the genVec struct itself, not the data
    // (which was transferred to dest)
    free(*src);
    *src = NULL;
}


void genVec_grow(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    u32 new_cap;
    if (vec->capacity < GENVEC_MIN_CAPACITY) {
        new_cap = vec->capacity + 1;
    } else {
        new_cap = (u32)((float)vec->capacity * GENVEC_GROWTH);
        if (new_cap <= vec->capacity) { // Ensure at least +1 growth
            new_cap = vec->capacity + 1;
        }
    }

    u8* new_data = realloc(vec->data.heap, GET_SCALED(vec, new_cap));
    CHECK_FATAL(!new_data, "realloc failed");

    vec->data.heap = new_data;
    vec->capacity  = new_cap;
}


void genVec_shrink(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    u32 reduced_cap = (u32)((float)vec->capacity * GENVEC_SHRINK_BY);
    if (reduced_cap < vec->size || reduced_cap == 0) {
        return;
    }

    u8* new_data = realloc(vec->data.heap, GET_SCALED(vec, reduced_cap));
    if (!new_data) {
        CHECK_WARN_RET(1, , "data realloc failed");
        return; // Keep original allocation
    }

    vec->data.heap = new_data;
    vec->capacity  = reduced_cap;
}

void genVec_migrate_to_heap(genVec* vec, u32 new_capacity)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!vec->svo, "vec already on heap");
    CHECK_FATAL(new_capacity <= vec->capacity, "new_capacity must be greater than old cap");

    u8* heap_data = (u8*)malloc(GET_SCALED(vec, new_capacity));
    CHECK_FATAL(!heap_data, "heap data malloc failed");

    memcpy(heap_data, vec->data.stack, GET_SCALED(vec, vec->size));

    vec->data.heap = heap_data;
    vec->capacity  = new_capacity;
    vec->svo       = false; // once set to false, can't be set to true again
}


#endif // GENVEC_SINGLE_H
       


#ifndef MAP_SETUP
#define MAP_SETUP

#include "common.h"
#include <string.h>


typedef enum {
    EMPTY = 0,
    FILLED = 1,
    TOMBSTONE = 2
} STATE;


typedef void (*copy_fn)(u8* dest, const u8* src);
typedef void (*move_fn)(u8* dest, u8** src);
typedef void (*delete_fn)(u8* key); 

typedef u32 (*custom_hash_fn)(const u8* key, u32 size);     
typedef int (*compare_fn)(const u8* a, const u8* b, u32 size);



#define LOAD_FACTOR_GROW 0.70
#define LOAD_FACTOR_SHRINK 0.20  
#define HASHMAP_INIT_CAPACITY 17  //prime no (index = hash % capacity)


/*
====================DEFAULT FUNCTIONS====================
*/
// 32-bit FNV-1a (default hash)
static u32 fnv1a_hash(const u8* bytes, u32 size) {
    u32 hash = 2166136261U;  // FNV offset basis

    for (u32 i = 0; i < size; i++) {
        hash ^= bytes[i];   // XOR with current byte
        hash *= 16777619U;  // Multiply by FNV prime
    }

    return hash;
}


// Default compare function
static int default_compare(const u8* a, const u8* b, u32 size) 
{
    return memcmp(a, b, size);
}

static const u32 PRIMES[] = {
    17, 37, 79, 163, 331, 673, 1361, 2729, 5471, 10949, 
    21911, 43853, 87719, 175447, 350899, 701819, 1403641, 
    2807303, 5614657, 11229331, 22458671, 44917381, 89834777
};

static const u32 PRIMES_COUNT = sizeof(PRIMES) / sizeof(PRIMES[0]);

// Find the next prime number larger than current
static u32 next_prime(u32 current) {
    for (u32 i = 0; i < PRIMES_COUNT; i++) {
        if (PRIMES[i] > current) {
            return PRIMES[i];
        }
    }
    
    // If we've exceeded our prime table, fall back to approximate prime
    // Using formula: next ≈ current * 2 + 1 (often prime or close to it)
    printf("Warning: exceeded prime table, using approximation\n");
    return (current * 2) + 1;
}

// Find the previous prime number smaller than current
static u32 prev_prime(u32 current) {
    // Search backwards through prime table
    for (u32 i = PRIMES_COUNT - 1; i >= 0; i--) {
        if (PRIMES[i] < current) {
            return PRIMES[i];
        }
    }
    
    // Should never happen if HASHMAP_INIT_CAPACITY is in our table
    printf("Warning: no smaller prime found\n");
    return HASHMAP_INIT_CAPACITY;
}



#endif // MAP_SETUP





typedef struct {
    genVec*         buckets;        // TODO: replace with raw array? (only using replace, get_ptr and init_val. memory managed by hashmap, not vec)
    u32             capacity;
    u32             size;
    u16             key_size;
    u16             val_size;
    custom_hash_fn  hash_fn;
    compare_fn      cmp_fn;
    copy_fn         key_copy_fn;
    move_fn         key_move_fn;
    delete_fn       key_del_fn;
    copy_fn         val_copy_fn;
    move_fn         val_move_fn;
    delete_fn       val_del_fn;
} hashmap;

/**
 * Create a new hashmap
 */
hashmap* hashmap_create(u16 key_size, u16 val_size, custom_hash_fn hash_fn,
                        compare_fn cmp_fn, copy_fn key_copy, copy_fn val_copy,
                        move_fn key_move, move_fn val_move,
                        delete_fn key_del, delete_fn val_del);

void hashmap_destroy(hashmap* map);

/**
 * Insert or update key-value pair (COPY semantics)
 * Both key and val are passed as const u8*
 * 
 * @return 1 if key existed (updated), 0 if new key inserted
 */
b8 hashmap_put(hashmap* map, const u8* key, const u8* val);

/**
 * Insert or update key-value pair (MOVE semantics)
 * Both key and val are passed as u8** and will be nulled
 * 
 * @return 1 if key existed (updated), 0 if new key inserted
 */
b8 hashmap_put_move(hashmap* map, u8** key, u8** val);

/**
 * Insert with mixed semantics
 */
b8 hashmap_put_val_move(hashmap* map, const u8* key, u8** val);
b8 hashmap_put_key_move(hashmap* map, u8** key, const u8* val);

/**
 * Get value for key (copy semantics)
 */
b8 hashmap_get(const hashmap* map, const u8* key, u8* val);

/**
 * Get pointer to value (no copy)
 * WARNING: Pointer invalidated by put/del operations
 */
u8* hashmap_get_ptr(hashmap* map, const u8* key);

/**
 * Delete key-value pair
 * If out is provided, value is copied to it before deletion
 */
b8 hashmap_del(hashmap* map, const u8* key, u8* out);


// TODO: 
void hashmap_clear(hashmap* map);  // Remove all, keep capacity
void hashmap_reset(hashmap* map);  // Remove all, reset to initial capacity
// Update value in-place if key exists, return false if key doesn't exist
b8 hashmap_update(hashmap* map, const u8* key, const u8* val);
genVec* hashmap_keys(const hashmap* map);
genVec* hashmap_values(const hashmap* map);

/**
 * Check if key exists
 */
b8 hashmap_has(const hashmap* map, const u8* key);

/**
 * Print all key-value pairs
 */
void hashmap_print(const hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print);

static inline u32 hashmap_size(const hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    return map->size;
}

static inline u32 hashmap_capacity(const hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    return map->capacity;
}

static inline b8 hashmap_empty(const hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    return map->size == 0;
}




typedef struct {
    u8* key;
    u8* val;
    STATE state;
} KV;

/*
====================KV HANDLERS====================
*/

static void kv_destroy(hashmap* map, const KV* kv)
{
    CHECK_FATAL(!kv, "kv is null");

    if (kv->key) {
        if (map->key_del_fn) {
            map->key_del_fn(kv->key); 
        }
        free(kv->key);
    }

    if (kv->val) {
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        free(kv->val);
    }
}

/*
====================PRIVATE FUNCTIONS====================
*/

static u32 find_slot(const hashmap* map, const u8* key,
                        b8* found, int* tombstone)
{
    u32 index = map->hash_fn(key, map->key_size) % map->capacity;

    *found = 0;
    *tombstone = -1;

    for (u32 x = 0; x < map->capacity; x++) 
    {
        u32 i = (index + x) % map->capacity;
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, i);

        switch (kv->state) {
            case EMPTY:
                return i;
            case FILLED:
                if (map->cmp_fn(kv->key, key, map->key_size) == 0) 
                {
                    *found = 1;
                    return i;
                }
                break;
            case TOMBSTONE:
                if (*tombstone == -1) {
                    *tombstone = (int)i;
                }
                break;
        }
    }
    
    return (*tombstone != -1) ? (u32)*tombstone : 0;
}

static void hashmap_resize(hashmap* map, u32 new_capacity) 
{
    if (new_capacity <= HASHMAP_INIT_CAPACITY) {
        new_capacity = HASHMAP_INIT_CAPACITY;
    }

    genVec* old_vec = map->buckets;

    KV kv = { .key = NULL, .val = NULL, .state = EMPTY };
    map->buckets = genVec_init_val(new_capacity, cast(kv), sizeof(KV), NULL, NULL, NULL);

    map->capacity = new_capacity;
    map->size = 0;

    for (u32 i = 0; i < old_vec->capacity; i++) {
        const KV* old_kv = (const KV*)genVec_get_ptr(old_vec, i);
        
        if (old_kv->state == FILLED) {
            b8 found = 0;
            int tombstone = -1;
            u32 slot = find_slot(map, old_kv->key, &found, &tombstone);

            KV new_kv = {
                .key = old_kv->key,
                .val = old_kv->val,
                .state = FILLED
            };

            genVec_replace(map->buckets, slot, (u8*)&new_kv);
            map->size++;
        }
    }

    genVec_destroy(old_vec);
}

static void hashmap_maybe_resize(hashmap* map) 
{
    CHECK_FATAL(!map, "map is null");
    
    double load_factor = (double)map->size / (double)map->capacity;
    
    if (load_factor > LOAD_FACTOR_GROW) {
        u32 new_cap = next_prime(map->capacity);
        hashmap_resize(map, new_cap);
    }
    else if (load_factor < LOAD_FACTOR_SHRINK && map->capacity > HASHMAP_INIT_CAPACITY) 
    {
        u32 new_cap = prev_prime(map->capacity);
        if (new_cap >= HASHMAP_INIT_CAPACITY) {
            hashmap_resize(map, new_cap);
        }
    }
}

/*
====================PUBLIC FUNCTIONS====================
*/

hashmap* hashmap_create(u16 key_size, u16 val_size, custom_hash_fn hash_fn,
                        compare_fn cmp_fn, copy_fn key_copy, copy_fn val_copy,
                        move_fn key_move, move_fn val_move,
                        delete_fn key_del, delete_fn val_del)
{
    CHECK_FATAL(key_size == 0, "key size can't be zero");
    CHECK_FATAL(val_size == 0, "val size can't be zero");

    hashmap* map = malloc(sizeof(hashmap));
    CHECK_FATAL(!map, "map malloc failed");

    KV kv = { 
        .key = NULL, 
        .val = NULL, 
        .state = EMPTY 
    };
    map->buckets = genVec_init_val(HASHMAP_INIT_CAPACITY, cast(kv), sizeof(KV), NULL, NULL, NULL);
    
    map->capacity = HASHMAP_INIT_CAPACITY;
    map->size = 0;
    map->key_size = key_size;
    map->val_size = val_size;

    map->hash_fn = hash_fn ? hash_fn : fnv1a_hash;
    map->cmp_fn = cmp_fn ? cmp_fn : default_compare;
    
    map->key_copy_fn = key_copy;
    map->key_move_fn = key_move;
    map->key_del_fn = key_del;
    
    map->val_copy_fn = val_copy;
    map->val_move_fn = val_move;
    map->val_del_fn = val_del;

    return map;
}

void hashmap_destroy(hashmap* map)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!map->buckets, "map bucket is null");

    for (u32 i = 0; i < map->capacity; i++) {
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, i);
        if (kv->state == FILLED) {
            kv_destroy(map, kv);
        }
    }
    genVec_destroy(map->buckets);
    free(map);
}

// COPY semantics - key and val are const u8*
b8 hashmap_put(hashmap* map, const u8* key, const u8* val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!val, "val is null");

    hashmap_maybe_resize(map);
    
    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);
    
    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);
        
        // Free old value's resources
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        
        // Update value
        if (map->val_copy_fn) {
            map->val_copy_fn(kv->val, val);
        } else {
            memcpy(kv->val, val, map->val_size);
        }
        
        return 1; // found - updated
    } 
    
    // New key - insert
    KV kv = {
        .key = malloc(map->key_size),
        .val = malloc(map->val_size),
        .state = FILLED
    };
    
    CHECK_FATAL(!kv.key, "key malloc failed");
    CHECK_FATAL(!kv.val, "val malloc failed");
    
    // Copy key
    if (map->key_copy_fn) {
        map->key_copy_fn(kv.key, key);
    } else {
        memcpy(kv.key, key, map->key_size);
    }
    
    // Copy value
    if (map->val_copy_fn) {
        map->val_copy_fn(kv.val, val);
    } else {
        memcpy(kv.val, val, map->val_size);
    }
    
    genVec_replace(map->buckets, slot, (u8*)&kv);
    map->size++;  
    
    return 0;
}

// MOVE semantics - key and val are u8**
b8 hashmap_put_move(hashmap* map, u8** key, u8** val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!*key, "*key is null");
    CHECK_FATAL(!val, "val is null");
    CHECK_FATAL(!*val, "*val is null");
    
    hashmap_maybe_resize(map);
    
    b8 found = 0;
    int tombstone = -1;
    // IMPORTANT: Dereference *key to pass u8* to find_slot
    u32 slot = find_slot(map, *key, &found, &tombstone);
    
    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);
        
        // Free old value's resources
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        
        // Move value
        if (map->val_move_fn) {
            map->val_move_fn(kv->val, val);
        } else {
            memcpy(kv->val, *val, map->val_size);
            *val = NULL;
        }
        
        // Key already exists, clean up the passed key
        if (map->key_del_fn) {
            map->key_del_fn(*key);
        }
        free(*key);
        *key = NULL;
        
        return 1;
    }
    
    // New key - insert with move semantics
    KV kv = {
        .key = malloc(map->key_size),
        .val = malloc(map->val_size),
        .state = FILLED
    };
    
    CHECK_FATAL(!kv.key, "key malloc failed");
    CHECK_FATAL(!kv.val, "val malloc failed");
    
    // Move key
    if (map->key_move_fn) {
        map->key_move_fn(kv.key, key);
    } else {
        memcpy(kv.key, *key, map->key_size);
        *key = NULL;
    }
    
    // Move value
    if (map->val_move_fn) {
        map->val_move_fn(kv.val, val);
    } else {
        memcpy(kv.val, *val, map->val_size);
        *val = NULL;
    }
    
    genVec_replace(map->buckets, slot, (u8*)&kv);
    map->size++;
    
    return 0;
}

// Mixed: key copy, val move
b8 hashmap_put_val_move(hashmap* map, const u8* key, u8** val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!val, "val is null");
    CHECK_FATAL(!*val, "*val is null");
    
    hashmap_maybe_resize(map);
    
    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);
    
    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);
        
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        
        if (map->val_move_fn) {
            map->val_move_fn(kv->val, val);
        } else {
            memcpy(kv->val, *val, map->val_size);
            *val = NULL;
        }
        
        return 1;
    }
    
    KV kv = {
        .key = malloc(map->key_size),
        .val = malloc(map->val_size),
        .state = FILLED
    };
    
    CHECK_FATAL(!kv.key, "key malloc failed");
    CHECK_FATAL(!kv.val, "val malloc failed");
    
    if (map->key_copy_fn) {
        map->key_copy_fn(kv.key, key);
    } else {
        memcpy(kv.key, key, map->key_size);
    }
    
    if (map->val_move_fn) {
        map->val_move_fn(kv.val, val);
    } else {
        memcpy(kv.val, *val, map->val_size);
        *val = NULL;
    }
    
    genVec_replace(map->buckets, slot, (u8*)&kv);
    map->size++;
    
    return 0;
}

// Mixed: key move, val copy
b8 hashmap_put_key_move(hashmap* map, u8** key, const u8* val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!*key, "*key is null");
    CHECK_FATAL(!val, "val is null");
    
    hashmap_maybe_resize(map);
    
    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, *key, &found, &tombstone);
    
    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);
        
        if (map->val_del_fn) {
            map->val_del_fn(kv->val);
        }
        
        if (map->val_copy_fn) {
            map->val_copy_fn(kv->val, val);
        } else {
            memcpy(kv->val, val, map->val_size);
        }
        
        if (map->key_del_fn) {
            map->key_del_fn(*key);
        }
        free(*key);
        *key = NULL;
        
        return 1;
    }
    
    KV kv = {
        .key = malloc(map->key_size),
        .val = malloc(map->val_size),
        .state = FILLED
    };
    
    CHECK_FATAL(!kv.key, "key malloc failed");
    CHECK_FATAL(!kv.val, "val malloc failed");
    
    if (map->key_move_fn) {
        map->key_move_fn(kv.key, key);
    } else {
        memcpy(kv.key, *key, map->key_size);
        *key = NULL;
    }
    
    if (map->val_copy_fn) {
        map->val_copy_fn(kv.val, val);
    } else {
        memcpy(kv.val, val, map->val_size);
    }
    
    genVec_replace(map->buckets, slot, (u8*)&kv);
    map->size++;
    
    return 0;
}

b8 hashmap_get(const hashmap* map, const u8* key, u8* val)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    CHECK_FATAL(!val, "val is null");
    
    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, slot);
        
        if (map->val_copy_fn) {
            map->val_copy_fn(val, kv->val);
        } else {
            memcpy(val, kv->val, map->val_size);
        }

        return 1;
    }

    return 0;
}

u8* hashmap_get_ptr(hashmap* map, const u8* key)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        return ((const KV*)genVec_get_ptr(map->buckets, slot))->val;
    } 

    return NULL;
}

b8 hashmap_del(hashmap* map, const u8* key, u8* out)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");

    if (map->size == 0) { return 0; }

    b8 found = 0;
    int tombstone = -1;
    u32 slot = find_slot(map, key, &found, &tombstone);

    if (found) {
        KV* kv = (KV*)genVec_get_ptr(map->buckets, slot);

        if (out) {
            if (map->val_copy_fn) {
                map->val_copy_fn(out, kv->val);
            } else {
                memcpy(out, kv->val, map->val_size);
            }
        }
        
        kv_destroy(map, kv);

        KV newkv = { .key = NULL, .val = NULL, .state = TOMBSTONE };
        genVec_replace(map->buckets, slot, (u8*)&newkv);
        map->size--;

        hashmap_maybe_resize(map);

        return 1;
    }

    return 0;
}

b8 hashmap_has(const hashmap* map, const u8* key)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key, "key is null");
    
    b8 found = 0;
    int tombstone = -1;
    find_slot(map, key, &found, &tombstone);
    
    return found;
}

void hashmap_print(const hashmap* map, genVec_print_fn key_print, genVec_print_fn val_print)
{
    CHECK_FATAL(!map, "map is null");
    CHECK_FATAL(!key_print, "key_print is null");
    CHECK_FATAL(!val_print, "val_print is null");

    printf("\t=========\n");
    printf("\tSize: %u / Capacity: %u\n", map->size, map->capacity);
    printf("\t=========\n");

    for (u32 i = 0; i < map->capacity; i++) {
        const KV* kv = (const KV*)genVec_get_ptr(map->buckets, i);
        if (kv->state == FILLED) {
            printf("\t");
            key_print(kv->key);
            printf(" => ");
            val_print(kv->val);
            printf("\n");
        }
    }

    printf("\t=========\n");
}






#endif // HASHMAP_SINGLE_H
