#ifndef QUEUE_H
#define QUEUE_H

#include "gen_vector.h"

// TODO: add copy/move to this?

typedef struct { // Circular Queue
    genVec* arr;
    u32 head;     // dequeue (head + 1) % capacity
    u32 tail;     // enqueue   (head + size) % capacity
    u32 size;
} Queue;


Queue* queue_create(u32 n, u16 data_size, genVec_delete_fn del_fn);
void queue_destroy(Queue* q);
void queue_clear(Queue* q);

void enqueue(Queue* q, const u8* x);
void dequeue(Queue* q, u8* out);
void queue_peek(Queue* q, u8* peek);

static inline u32 queue_size(Queue* q) {
    return q->size;
}

static inline u8 queue_empty(Queue* q) {
    return q->size == 0;
}

void queue_print(Queue* q, genVec_print_fn print_fn);



#endif // QUEUE_H
