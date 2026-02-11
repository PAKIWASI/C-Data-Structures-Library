#include "gen_array.h"
#include "common.h"
#include <string.h>


// MACROS

// Get ptr to element at index i
#define GET_PTR(arr, i) ((arr->data) + ((u64)(i) * ((arr)->data_size)))

// Get total size in bytes for i elements
#define GET_SCALED(arr, i) ((u64)(i) * ((arr)->data_size))



// Memory Management
// ===========================

genArr* genarr_init(u64 size, u32 data_size)
{
    CHECK_FATAL(size == 0, "arr size can't be 0");
    CHECK_FATAL(data_size == 0, "data size can't be 0");

    genArr* arr = malloc(sizeof(genArr));
    CHECK_FATAL(!arr, "arr malloc failed");

    arr->data = malloc(size * data_size);
    CHECK_FATAL(!arr->data, "arr data malloc failed");

    arr->data_size = data_size;
    arr->size      = size;

    return arr;
}



genArr* genarr_init_val(u64 size, const u8* val, u32 data_size)
{
    CHECK_FATAL(!val, "val can't be null");
    CHECK_FATAL(size == 0, "size can't be 0");

    genArr* arr = genarr_init(size, data_size);

    // Fill array with val
    for (u64 i = 0; i < size; i++) {
        memcpy(GET_PTR(arr, i), val, data_size);
    }

    return arr;
}


void genarr_init_stk(genArr* arr, u8* stk_data, u64 size, u32 data_size)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(!stk_data, "stk_data is null");
    CHECK_FATAL(size == 0, "arr size can't be 0");
    CHECK_FATAL(data_size == 0, "data size can't be 0");

    arr->data      = stk_data;
    arr->data_size = data_size;
    arr->size      = size;
}


void genarr_destroy(genArr* arr)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(!arr->data, "arr data is null");

    free(arr->data);
    free(arr);
}



void genarr_fill(genArr* arr, const u8* val)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(!val, "val is null");

    for (u64 i = 0; i < arr->size; i++) {
        memcpy(GET_PTR(arr, i), val, arr->data_size);
    }
}


void genarr_fill_range(genArr* arr, u64 start, u64 end, const u8* val)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(!val, "val is null");
    CHECK_FATAL(start >= arr->size, "start index out of bounds");
    CHECK_FATAL(end > arr->size, "end index out of bounds");
    CHECK_FATAL(start > end, "invalid range: start > end");

    for (u64 i = start; i < end; i++) {
        memcpy(GET_PTR(arr, i), val, arr->data_size);
    }
}



// Operations
// ===========================

void genarr_get(const genArr* arr, u64 i, u8* out)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(!out, "out is null");
    CHECK_FATAL(i >= arr->size, "index out of bounds");

    memcpy(out, GET_PTR(arr, i), arr->data_size);
}


const u8* genarr_get_ptr(const genArr* arr, u64 i)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(i >= arr->size, "index out of bounds");

    return GET_PTR(arr, i);
}


u8* genarr_get_ptr_mut(genArr* arr, u64 i)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(i >= arr->size, "index out of bounds");

    return GET_PTR(arr, i);
}


void genarr_set(genArr* arr, u64 i, const u8* data)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(!data, "data is null");
    CHECK_FATAL(i >= arr->size, "index out of bounds");

    memcpy(GET_PTR(arr, i), data, arr->data_size);
}


void genarr_swap(genArr* arr, u64 i, u64 j)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(i >= arr->size, "index i out of bounds");
    CHECK_FATAL(j >= arr->size, "index j out of bounds");

    if (i == j) {
        return;
    }

    // Allocate temporary buffer
    u8* temp = malloc(arr->data_size);
    CHECK_FATAL(!temp, "temp malloc failed");

    u8* ptr_i = GET_PTR(arr, i);
    u8* ptr_j = GET_PTR(arr, j);

    // Swap using temp buffer
    memcpy(temp, ptr_i, arr->data_size);
    memcpy(ptr_i, ptr_j, arr->data_size);
    memcpy(ptr_j, temp, arr->data_size);

    free(temp);
}


void genarr_copy_range(genArr* dest, u64 dest_start, const genArr* src, u64 src_start, u64 count)
{
    CHECK_FATAL(!dest, "dest is null");
    CHECK_FATAL(!src, "src is null");
    CHECK_FATAL(dest->data_size != src->data_size, "data_size mismatch");
    CHECK_FATAL(dest_start + count > dest->size, "dest range out of bounds");
    CHECK_FATAL(src_start + count > src->size, "src range out of bounds");

    for (u64 i = 0; i < count; i++) {
        memcpy(GET_PTR(dest, dest_start + i), GET_PTR(src, src_start + i), src->data_size);
    }
}


void genarr_for_each(genArr* arr, for_each_fn for_each)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(!for_each, "for_each function is null");

    for (u64 i = 0; i < arr->size; i++) {
        for_each(GET_PTR(arr, i));
    }
}


const u8* genarr_front(const genArr* arr)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(arr->size == 0, "arr is empty");

    return arr->data;
}


const u8* genarr_back(const genArr* arr)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(arr->size == 0, "arr is empty");

    return GET_PTR(arr, arr->size - 1);
}



void genarr_reverse(genArr* arr)
{
    CHECK_FATAL(!arr, "arr is null");

    if (arr->size <= 1) {
        return;
    }

    u8* temp = malloc(arr->data_size);
    CHECK_FATAL(!temp, "temp malloc failed");

    u64 left  = 0;
    u64 right = arr->size - 1;

    while (left < right) {
        u8* ptr_left  = GET_PTR(arr, left);
        u8* ptr_right = GET_PTR(arr, right);

        // Swap using temp buffer
        memcpy(temp, ptr_left, arr->data_size);
        memcpy(ptr_left, ptr_right, arr->data_size);
        memcpy(ptr_right, temp, arr->data_size);

        left++;
        right--;
    }

    free(temp);
}



// Utility
// ===========================

void genarr_print(const genArr* arr, print_fn print_fn)
{
    CHECK_FATAL(!arr, "arr is null");
    CHECK_FATAL(!print_fn, "print func is null");

    printf("[ ");
    for (u64 i = 0; i < arr->size; i++) {
        print_fn(GET_PTR(arr, i));
        putchar(' ');
    }
    putchar(']');
}


void genarr_copy(genArr* dest, const genArr* src)
{
    CHECK_FATAL(!dest, "dest is null");
    CHECK_FATAL(!src, "src is null");

    if (dest == src) {
        return;
    }

    // Clean up dest if already initialized
    // TODO:  can't work for stk based
    if (dest->data) {
        free(dest->data);
    }

    // Copy all fields
    memcpy(dest, src, sizeof(genArr));

    // Allocate new data
    dest->data = malloc(GET_SCALED(src, src->size));
    CHECK_FATAL(!dest->data, "dest data malloc failed");

    // Copy elements (simple memcpy for simple types)
    memcpy(dest->data, src->data, GET_SCALED(src, src->size));
}




