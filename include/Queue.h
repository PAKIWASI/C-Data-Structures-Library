#ifndef QUEUE_H
#define QUEUE_H

#include "gen_vector.h"


typedef struct { // Circular Queue
    genVec* arr;
    u32 head;   // dequeue (head + 1) % capacity
    u32 tail;   // enqueue (head + size) % capacity
    u32 size;     
} Queue;


Queue* queue_create(u32 n, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn);
Queue* queue_create_val(u32 n, const u8* val, u16 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn);
void queue_destroy(Queue* q);
void queue_clear(Queue* q);
void queue_reset(Queue* q);
void queue_shrink_to_fit(Queue* q);

void enqueue(Queue* q, const u8* x);
void enqueue_move(Queue* q, u8** x);
void dequeue(Queue* q, u8* out);
void queue_peek(Queue* q, u8* peek);
const u8* queue_peek_ptr(Queue* q);


static inline u32 queue_size(Queue* q) {
    CHECK_FATAL(!q, "queue is null");
    return q->size;
}

static inline u8 queue_empty(Queue* q) {
    CHECK_FATAL(!q, "queue is null");
    return q->size == 0;
}

static inline u32 queue_capacity(Queue* q) {
    CHECK_FATAL(!q, "queue is null");
    CHECK_FATAL(!q->arr, "queue->arr is null");
    return genVec_capacity(q->arr);
}

void queue_print(Queue* q, genVec_print_fn print_fn);

#endif // QUEUE_H
