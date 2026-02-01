
#ifndef GEN_VECTOR_SINGLE_H
#define GEN_VECTOR_SINGLE_H


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

void print_hex(const u8* ptr, u32 size, u32 bytes_per_line);

#endif // COMMON_H


// === HEADER FILE ===


// User-provided callback functions
typedef void (*genVec_print_fn)(const u8* elm);
typedef b8   (*genVec_compare_fn)(const u8* a, const u8* b); 
typedef void (*genVec_delete_fn)(u8* elm);                 // Cleanup owned resources
typedef void (*genVec_copy_fn)(u8* dest, const u8* src);   // Deep copy resources
typedef void (*genVec_move_fn)(u8* dest, u8** src);         // Move src into dest, null src


// genVec growth/shrink settings (define your own before including this file)
//
#define GROWTH    1.5  // vec capacity multiplier
#define SHRINK_AT 0.25 // % filled to shrink at (25% filled)
#define SHRINK_BY 0.5  // capacity dividor (half)
                      

// generic vector container
typedef struct {
    u8*              data;      // Contiguous array of elements
    u32              size;      // Number of elements currently in vector
    u32              capacity;  // Total allocated capacity
    u16              data_size; // Size of each element in bytes
    genVec_copy_fn   copy_fn;   // Deep copy function for owned resources (or NULL)
    genVec_move_fn   move_fn;   // Get a double pointer, transfer ownership and null original
    genVec_delete_fn del_fn;    // Cleanup function for owned resources (or NULL)
} genVec;


// Memory Management
// ===========================

// Initialize vector with capacity n. If elements own heap resources,
// provide copy_fn (deep copy) and del_fn (cleanup). Otherwise pass NULL.
genVec* genVec_init(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn);

// Initialize vector on stack with data on heap
void genVec_init_stk(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, 
                     genVec_delete_fn del_fn, genVec* vec);

// Initialize vector of size n, all elements set to val
genVec* genVec_init_val(u32 n, const u8* val, u16 data_size, genVec_copy_fn copy_fn,
                        genVec_move_fn move_fn, genVec_delete_fn del_fn);

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
void genVec_reserve_val(genVec* vec, u32 new_capacrity, const u8* val);


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


// === C FILE ===

#include <string.h>



#define GENVEC_MIN_CAPACITY 4


// get ptr to elm at index i
#define GET_PTR(vec, i) (((vec)->data) + ((size_t)(i) * ((vec)->data_size)))
// get total_size in bytes for i elements
#define GET_SCALED(vec, i) ((size_t)(i) * ((vec)->data_size)) // TODO: potential overflow?



//private functions
void genVec_grow(genVec* vec);
void genVec_shrink(genVec* vec);


genVec* genVec_init(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn,
                    genVec_delete_fn del_fn)
{
    CHECK_FATAL(data_size == 0, "data_size can't be 0");

    genVec* vec = malloc(sizeof(genVec));
    CHECK_FATAL(!vec, "vec init failed");

    // Only allocate memory if n > 0, otherwise data can be NULL
    vec->data = (n > 0) ? malloc((size_t)data_size * n) : NULL;

    // Only check for allocation failure if we actually tried to allocate
    if (n > 0 && !vec->data) {
        free(vec);
        FATAL("data init failed");
    }

    vec->size      = 0;
    vec->capacity  = n;
    vec->data_size = data_size;

    vec->copy_fn = copy_fn;
    vec->move_fn = move_fn;
    vec->del_fn  = del_fn;

    return vec;
}


void genVec_init_stk(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn,
                     genVec_delete_fn del_fn, genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(data_size == 0, "data_size can't be 0");

    // Only allocate memory if n > 0, otherwise data can be NULL
    vec->data = (n > 0) ? malloc((size_t)data_size * n) : NULL;
    CHECK_FATAL(n > 0 && !vec->data, "data init failed");

    vec->size      = 0;
    vec->capacity  = n;
    vec->data_size = data_size;
    vec->copy_fn   = copy_fn;
    vec->move_fn   = move_fn;
    vec->del_fn    = del_fn;
}


genVec* genVec_init_val(u32 n, const u8* val, u16 data_size, genVec_copy_fn copy_fn,
                        genVec_move_fn move_fn, genVec_delete_fn del_fn)
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

void genVec_destroy(genVec* vec)
{
    genVec_destroy_stk(vec);

    free(vec);
}

void genVec_destroy_stk(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (!vec->data) { return; } // TODO: check this

    if (vec->del_fn) {
        // Custom cleanup for each element
        for (u32 i = 0; i < vec->size; i++) { 
            vec->del_fn(GET_PTR(vec, i)); 
        }
    }

    free(vec->data);
    vec->data = NULL;
    // dont free vec as on stk (don't own memory)
}

void genVec_clear(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (vec->del_fn) { // owns resources
        for (u32 i = 0; i < vec->size; i++) { vec->del_fn(GET_PTR(vec, i)); }
    }
    // doesn't free container
    vec->size = 0;
}

void genVec_reset(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (vec->del_fn) {
        for (u32 i = 0; i < vec->size; i++) { vec->del_fn(GET_PTR(vec, i)); }
    }

    free(vec->data);
    vec->data = NULL;

    vec->size     = 0;
    vec->capacity = 0;
}


void genVec_reserve(genVec* vec, u32 new_capacity)
{
    CHECK_FATAL(!vec, "vec is null");

    // Only grow, never shrink with reserve
    if (new_capacity <= vec->capacity) { return; }

    u8* new_data = realloc(vec->data, GET_SCALED(vec, new_capacity));
    CHECK_FATAL(!new_data, "realloc failed");

    vec->data     = new_data;
    vec->capacity = new_capacity;
}


void genVec_reserve_val(genVec* vec, u32 new_capacrity, const u8* val)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!val, "val is null");
    CHECK_FATAL(new_capacrity < vec->size, "new_capacity must be >= current size");

    genVec_reserve(vec, new_capacrity);

    for (u32 i = vec->size; i < new_capacrity; i++) {
        if (vec->copy_fn) {
            vec->copy_fn(GET_PTR(vec, i), val);
        } else {
            memcpy(GET_PTR(vec, i), val, vec->data_size);
        }
    }

    vec->size = new_capacrity;
}


void genVec_push(genVec* vec, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");

    // Check if we need to allocate or grow
    if (vec->size >= vec->capacity || !vec->data) { genVec_grow(vec); }

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
    if (vec->size >= vec->capacity || !vec->data) { genVec_grow(vec); }

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

    if (vec->size <= (u32)((double)vec->capacity * SHRINK_AT)) { genVec_shrink(vec); }
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


void genVec_insert(genVec* vec, u32 i, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(!data, "data is null");

    CHECK_FATAL(i > vec->size, "index out of bounds");

    if (i == vec->size) {
        genVec_push(vec, data);
        return;
    }

    // Check if we need to allocate or grow
    if (vec->size >= vec->capacity || !vec->data) { genVec_grow(vec); }

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

    // Check if we need to allocate or grow
    if (vec->size >= vec->capacity || !vec->data) { genVec_grow(vec); }

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

        memmove(dest, src,
                GET_SCALED(vec, elements_to_shift)); // using memmove for overlapping regions
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

        memmove(dest, src,
                GET_SCALED(vec, elements_to_shift)); // using memmove for overlapping regions
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

    if (vec->del_fn) { vec->del_fn(GET_PTR(vec, i)); }
    // Calculate the number of elements to shift
    u32 elements_to_shift = vec->size - i - 1;

    if (elements_to_shift > 0) {
        // Shift elements left to overwrite the deleted element
        u8* dest = GET_PTR(vec, i);
        u8* src  = GET_PTR(vec, i + 1);

        memmove(dest, src,
                GET_SCALED(vec, elements_to_shift)); // Use memmove for overlapping regions
    }

    vec->size--;

    if (vec->size <= (u32)((double)vec->capacity * SHRINK_AT)) { genVec_shrink(vec); }
}


void genVec_remove_range(genVec* vec, u32 l, u32 r)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(l >= vec->size, "index out of range");
    CHECK_FATAL(l > r, "invalid range");

    if (r >= vec->size) { r = vec->size - 1; }

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

    if (vec->size <= (u32)((double)vec->capacity * SHRINK_AT)) { genVec_shrink(vec); }
}


void genVec_replace(genVec* vec, u32 i, const u8* data)
{
    CHECK_FATAL(!vec, "vec is null");
    CHECK_FATAL(i >= vec->size, "index out of bounds");
    CHECK_FATAL(!data, "need a valid data variable");

    u8* to_replace = GET_PTR(vec, i);

    if (vec->del_fn) { vec->del_fn(to_replace); }

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

    if (vec->del_fn) { vec->del_fn(to_replace); }

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

    return vec->data;
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
        u8* element = GET_PTR(vec, i);
        fn(element);
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

    if (dest == src) { return; }

    // if data ptr is null, no op
    genVec_destroy_stk(dest);

    // copy all fields
    memcpy(dest, src, sizeof(genVec));

    // malloc data ptr
    dest->data = malloc(GET_SCALED(src, src->capacity));

    // Copy elements
    if (src->copy_fn) {
        for (u32 i = 0; i < src->size; i++) {
            src->copy_fn(GET_PTR(dest, i), GET_PTR(src, i)); 
        }
    } else {
        memcpy(dest->data, src->data, GET_SCALED(src, src->size));
    }
}


void genVec_move(genVec* dest, genVec** src)
{
    CHECK_FATAL(!src, "src is null");
    CHECK_FATAL(!*src, "*src is null");
    CHECK_FATAL(!dest, "dest is null");

    if (dest == *src) {
        *src = NULL;
        return;
    }

    // Transfer all fields from src to dest
    memcpy(dest, *src, sizeof(genVec));

    // Null out src's data pointer so it doesn't get freed
    (*src)->data = NULL;

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
        new_cap = (u32)((double)vec->capacity * GROWTH);
        if (new_cap <= vec->capacity) { // Ensure at least +1 growth
            new_cap = vec->capacity + 1;
        }
    }

    u8* new_data = realloc(vec->data, GET_SCALED(vec, new_cap));
    CHECK_FATAL(!new_data, "realloc failed");

    vec->data     = new_data;
    vec->capacity = new_cap;
}


void genVec_shrink(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    u32 reduced_cap = (u32)((double)vec->capacity * SHRINK_BY);
    if (reduced_cap < vec->size || reduced_cap == 0) { return; }

    u8* new_data = realloc(vec->data, GET_SCALED(vec, reduced_cap));
    if (!new_data) {
        CHECK_WARN_RET(1, , "data realloc failed");
        return; // Keep original allocation
    }

    vec->data     = new_data;
    vec->capacity = reduced_cap;
}


#endif // GEN_VECTOR_SINGLE_H
