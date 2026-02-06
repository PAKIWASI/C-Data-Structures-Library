#include "Queue.h"
#include <stdlib.h>
#include <string.h>


#define QUEUE_GROWTH    1.5
#define QUEUE_SHRINK_AT 0.25 // Shrink when less than 25% full
#define QUEUE_SHRINK_BY 0.5  // Reduce capacity by half when shrinking


static void tail_update(Queue* q);
static void queue_grow(Queue* q);
static void queue_shrink(Queue* q);
static void queue_maybe_shrink(Queue* q);
static void queue_compact(Queue* q, u32 new_capacity);


Queue* queue_create(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn)
{
    CHECK_FATAL(n == 0, "n can't be 0");
    CHECK_FATAL(data_size == 0, "data_size can't be 0");

    Queue* q = malloc(sizeof(Queue));
    CHECK_FATAL(!q, "queue malloc failed");

    q->arr = genVec_init(n, data_size, copy_fn, move_fn, del_fn);

    q->head = 0; // (head + 1) % capacity
    q->tail = 0; // (head + size) % capacity
    q->size = 0;

    return q;
}

Queue* queue_create_val(u32 n, const u8* val, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn,
                        genVec_delete_fn del_fn)
{
    CHECK_FATAL(n == 0, "n can't be 0");
    CHECK_FATAL(data_size == 0, "data_size can't be 0");
    CHECK_FATAL(!val, "val is null");

    Queue* q = malloc(sizeof(Queue));
    CHECK_FATAL(!q, "queue malloc failed");

    q->arr = genVec_init_val(n, val, data_size, copy_fn, move_fn, del_fn);

    q->head = 0; // (head + 1) % capacity
    q->tail = 0; // (head + size) % capacity
    q->size = n; // all n elements are initialized with val

    return q;
}


void queue_destroy(Queue* q)
{
    CHECK_FATAL(!q, "queue is null");

    genVec_destroy(q->arr);
    free(q);
}

void queue_clear(Queue* q)
{
    CHECK_FATAL(!q, "queue is null");

    genVec_clear(q->arr);
    q->size = 0;
    q->head = 0;
    q->tail = 0;
}

void queue_reset(Queue* q)
{
    CHECK_FATAL(!q, "queue is null");

    genVec_reset(q->arr);
    q->size = 0;
    q->head = 0;
    q->tail = 0;
}

void enqueue(Queue* q, const u8* x)
{
    CHECK_FATAL(!q, "queue is null");
    CHECK_FATAL(!x, "x is null");

    // If queue is full, grow it
    if (q->size == genVec_capacity(q->arr)) {
        queue_grow(q);
    }

    genVec_replace(q->arr, q->tail, x);
    q->size++;
    tail_update(q);
}

void enqueue_move(Queue* q, u8** x)
{
    CHECK_FATAL(!q, "queue is null");
    CHECK_FATAL(!x, "x is null");
    CHECK_FATAL(!*x, "*x is null");

    // If queue is full, grow it
    if (q->size == genVec_capacity(q->arr)) {
        queue_grow(q);
    }

    genVec_replace_move(q->arr, q->tail, x);
    q->size++;
    tail_update(q);
}

void dequeue(Queue* q, u8* out)
{
    CHECK_FATAL(!q, "queue is null");

    CHECK_WARN_RET(q->size == 0, , "can't dequeue empty queue");

    if (out) {
        genVec_get(q->arr, q->head, out);
    }

    // Create dummy element to clear the position
    u8* dummy = malloc(q->arr->data_size);
    CHECK_FATAL(!dummy, "dummy malloc failed");

    // Use replace to properly clean up the old element
    genVec_replace(q->arr, q->head, dummy);

    free(dummy);

    q->head = (q->head + 1) % genVec_capacity(q->arr);
    q->size--;

    // Check if we should shrink
    queue_maybe_shrink(q);
}

void dequeue_move(Queue* q, u8** out)
{
    CHECK_FATAL(!q, "queue is null");
    CHECK_FATAL(!out, "out is null");
    CHECK_WARN_RET(q->size == 0, , "can't dequeue empty queue");

    // Move the element to output
    if (*out) {
        FATAL("out pointer must be NULL for move semantics");
    }

    // Allocate memory for output
    *out = malloc(q->arr->data_size);
    CHECK_FATAL(!*out, "out malloc failed");

    // Get the element at head
    const u8* src = genVec_get_ptr(q->arr, q->head);

    // Move the element based on move_fn
    if (q->arr->move_fn) {
        // We need to create a temporary pointer that we can modify
        u8* temp_src = (u8*)src;
        q->arr->move_fn(*out, &temp_src);
        // Clear the source after move
        if (temp_src) {
            memset(temp_src, 0, q->arr->data_size);
        }
    } else {
        memcpy(*out, src, q->arr->data_size);
        // Clear the original position
        memset((u8*)src, 0, q->arr->data_size);
    }

    // Create dummy element to clear the position
    u8* dummy = malloc(q->arr->data_size);
    CHECK_FATAL(!dummy, "dummy malloc failed");

    genVec_replace(q->arr, q->head, dummy);
    free(dummy);

    q->head = (q->head + 1) % genVec_capacity(q->arr);
    q->size--;

    // Check if we should shrink
    queue_maybe_shrink(q);
}

void queue_peek(Queue* q, u8* peek)
{
    CHECK_FATAL(!q, "queue is null");
    CHECK_FATAL(!peek, "peek is null");

    CHECK_WARN_RET(q->size == 0, , "can't peek at empty queue");

    genVec_get(q->arr, q->head, peek);
}

const u8* queue_peek_ptr(Queue* q)
{
    CHECK_FATAL(!q, "queue is null");

    CHECK_WARN_RET(q->size == 0, NULL, "can't peek at empty queue");

    return genVec_get_ptr(q->arr, q->head);
}

void queue_print(Queue* q, genVec_print_fn print_fn)
{
    CHECK_FATAL(!q, "queue is empty");
    CHECK_FATAL(!print_fn, "print_fn is empty");

    u32 h   = q->head;
    u32 cap = genVec_capacity(q->arr);

    printf("[ ");

    if (q->size != 0) {
        for (u32 i = 0; i < q->size; i++) {
            const u8* out = genVec_get_ptr(q->arr, h);
            print_fn(out);
            printf(" ");
            h = (h + 1) % cap;
        }
    }

    printf("]\n");
}

// Manual shrink function
void queue_shrink_to_fit(Queue* q)
{
    CHECK_FATAL(!q, "queue is null");

    if (q->size == 0) {
        queue_reset(q);
        return;
    }

    // Don't shrink below minimum useful capacity
    u32 min_capacity     = q->size > 4 ? q->size : 4;
    u32 current_capacity = genVec_capacity(q->arr);

    if (current_capacity > min_capacity) {
        queue_compact(q, min_capacity);
    }
}

static void tail_update(Queue* q)
{
    q->tail = (q->head + q->size) % genVec_capacity(q->arr);
}

static void queue_grow(Queue* q)
{
    u32 old_cap = genVec_capacity(q->arr);
    u32 new_cap = (u32)((float)old_cap * QUEUE_GROWTH);
    if (new_cap <= old_cap) {
        new_cap = old_cap + 1;
    }

    queue_compact(q, new_cap);
}

static void queue_shrink(Queue* q)
{
    u32 current_cap = genVec_capacity(q->arr);
    u32 new_cap     = (u32)((float)current_cap * QUEUE_SHRINK_BY);

    // Don't shrink below current size or minimum capacity
    u32 min_capacity = q->size > 4 ? q->size : 4;
    if (new_cap < min_capacity) {
        new_cap = min_capacity;
    }

    // Only shrink if we're actually reducing capacity
    if (new_cap < current_cap) {
        queue_compact(q, new_cap);
    }
}

static void queue_maybe_shrink(Queue* q)
{
    u32 capacity = genVec_capacity(q->arr);

    // Don't shrink if we're at or near minimum capacity
    if (capacity <= 4) {
        return;
    }

    // Calculate load factor
    float load_factor = (float)q->size / (float)capacity;

    // Shrink if load factor is too low
    if (load_factor < QUEUE_SHRINK_AT) {
        queue_shrink(q);
    }
}

static void queue_compact(Queue* q, u32 new_capacity)
{
    CHECK_FATAL(new_capacity < q->size, "new_capacity must be >= current size");

    // Create a new vector with the desired capacity
    genVec* new_arr = genVec_init(new_capacity, q->arr->data_size, q->arr->copy_fn, q->arr->move_fn, q->arr->del_fn);

    // Copy elements from old queue to new queue in order
    u32 h       = q->head;
    u32 old_cap = genVec_capacity(q->arr);

    for (u32 i = 0; i < q->size; i++) {
        const u8* elem = genVec_get_ptr(q->arr, h);

        if (q->arr->copy_fn) {
            genVec_push(new_arr, elem);
        } else {
            // For move semantics, we need to move each element
            if (q->arr->move_fn) {
                // Create a pointer we can modify
                u8* temp_elem = (u8*)elem;
                genVec_push_move(new_arr, &temp_elem);
            } else {
                genVec_push(new_arr, elem);
            }
        }

        h = (h + 1) % old_cap;
    }

    // If we used move semantics, we need to clear the old array
    if (q->arr->move_fn) {
        // Clear the old array (elements have been moved)
        for (u32 i = 0; i < q->size; i++) {
            u32 idx  = (q->head + i) % old_cap;
            u8* elem = (u8*)genVec_get_ptr(q->arr, idx);
            if (q->arr->del_fn) {
                q->arr->del_fn(elem);
            }
            memset(elem, 0, q->arr->data_size);
        }
    }

    // Replace the old array with the new one
    genVec_destroy(q->arr);
    q->arr = new_arr;

    // Reset head and tail for the new compacted array
    q->head = 0;
    q->tail = q->size % new_capacity; // tail is 0 if empty, size if not empty
}


