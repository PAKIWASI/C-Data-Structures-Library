#ifndef GEN_ARRAY_H
#define GEN_ARRAY_H

#include "common.h"


// TODO: make it macro based

/*          TLDR
 * genArr is a fixed-size generic array for simple types (int, float, etc.).
 * Elements are stored inline by value (no memory ownership).
 *
 * Unlike genVec, genArr:
 * - Has fixed capacity (cannot grow)
 * - No copy/move/delete functions (simple types only)
 * - Use for stack-allocated arrays or when size is known at compile-time
 */


// generic static array (fixed size, simple types only)
typedef struct {
    u8* data;      // pointer to generic data
    u64 size;      // Total size/capacity (fixed)
    u32 data_size; // Size of each element in bytes
} genArr;



// Memory Management
// ===========================

// Initialize heap-allocated array with fixed size n
genArr* genarr_init(u64 size, u32 data_size);

// Initialize array with all elements set to val
genArr* genarr_init_val(u64 size, const u8* val, u32 data_size);

// Initialize array COMPLETELY on stack (arr + data)
// You provide a stack-initialized array which becomes the internal data
void genarr_init_stk(genArr* arr, u8* stk_data, u64 size, u32 data_size);

// Destroy heap-allocated array
void genarr_destroy(genArr* arr);

// Fill entire array with val
void genarr_fill(genArr* arr, const u8* val);

// Fill range [start, end) with val
void genarr_fill_range(genArr* arr, u64 start, u64 end, const u8* val);



// Operations
// ===========================

// Copy element at index i into out buffer
void genarr_get(const genArr* arr, u64 i, u8* out);

// Get pointer to element at index i (read-only)
const u8* genarr_get_ptr(const genArr* arr, u64 i);

// Get mutable pointer to element at index i
u8* genarr_get_ptr_mut(genArr* arr, u64 i);

// Set element at index i to data (copy)
void genarr_set(genArr* arr, u64 i, const u8* data);

// Swap two elements at indices i and j
void genarr_swap(genArr* arr, u64 i, u64 j);

// Copy range [src_start, src_start+count) from src to dest starting at dest_start
void genarr_copy_range(genArr* dest, u64 dest_start, const genArr* src, u64 src_start, u64 count);

// Apply a function to each element
void genarr_for_each(genArr* arr, for_each_fn for_each);

// Get pointer to first element
const u8* genarr_front(const genArr* arr);

// Get pointer to last element
const u8* genarr_back(const genArr* arr);

// Reverse the array in-place
void genarr_reverse(genArr* arr);



// Utility
// ===========================

// Print all elements using provided print function
void genarr_print(const genArr* arr, print_fn fn);

// Deep copy src array into dest
// Note: works for heap allocated array
void genarr_copy(genArr* dest, const genArr* src);




#endif // GEN_ARRAY_H
