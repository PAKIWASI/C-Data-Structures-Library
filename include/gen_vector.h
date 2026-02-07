#ifndef GEN_VECTOR_H
#define GEN_VECTOR_H

#include "common.h"


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


// TODO: can i define this in C file and still be user configurable?
//
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

// TODO: make svo size dynamic by making it depend on data_size ? ...maybe size / data_size ?



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


// TODO: can i do a genVec with union only containing heap (less size) ?
// I could use that one for things that are completely on the heap
// and the api whould be same, it's svo flag would be always false


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

// Shrink vector to it's size
void genVec_shrink_to_fit(genVec* vec);



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


// TODO: iterator support ?
// TODO: add:
/*

// Resize vector to exact size (useful after bulk operations)
void genVec_shrink_to_fit(genVec* vec);

// Extend with multiple copies of val
void genVec_extend(genVec* vec, const u8* val, u32 count);

// Swap two elements
void genVec_swap(genVec* vec, u32 i, u32 j);

// Find element index (-1 if not found)
i32 genVec_find(const genVec* vec, const u8* elm, genVec_compare_fn cmp);

// Reverse the vector in-place
void genVec_reverse(genVec* vec);

// Filter in-place (remove elements where predicate returns false)
void genVec_filter(genVec* vec, b8 (*predicate)(const u8*));

*/

/*

void genVec_shrink_to_fit(genVec* vec)
{
    CHECK_FATAL(!vec, "vec is null");

    if (vec->size == 0) {
        genVec_reset(vec);
        return;
    }

    // Don't shrink below minimum useful capacity
    u32 min_capacity = vec->size > GENVEC_MIN_CAPACITY ? vec->size : GENVEC_MIN_CAPACITY;
    u32 current_capacity = vec->capacity;

    if (current_capacity <= min_capacity) {
        return; // Already at or below target size
    }

    // Calculate SVO capacity
    u32 svo_cap = GENVEC_SVO_CAPACITY(vec->data_size);

    // Case 1: Currently in SVO mode and target fits in SVO
    if (vec->svo && min_capacity <= svo_cap) {
        vec->capacity = svo_cap;
        return;
    }

    // Case 2: Currently in SVO mode but target needs heap
    if (vec->svo && min_capacity > svo_cap) {
        genVec_migrate_to_heap(vec, min_capacity);
        return;
    }

    // Case 3: Currently on heap, target fits in SVO - migrate back to stack
    if (!vec->svo && min_capacity <= svo_cap) {
        u8 stack_backup[GENVEC_SVO_SIZE];
        memcpy(stack_backup, vec->data.heap, GET_SCALED(vec, vec->size));
        free(vec->data.heap);
        memcpy(vec->data.stack, stack_backup, GET_SCALED(vec, vec->size));
        vec->svo = true;
        vec->capacity = svo_cap;
        return;
    }

    // Case 4: Currently on heap, target stays on heap - just realloc
    u8* new_data = realloc(vec->data.heap, GET_SCALED(vec, min_capacity));
    CHECK_FATAL(!new_data, "realloc failed in shrink_to_fit");
    vec->data.heap = new_data;
    vec->capacity = min_capacity;
}
*/
#endif // GEN_VECTOR_H
