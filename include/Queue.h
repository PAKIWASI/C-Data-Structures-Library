#pragma once

#include "gen_vector.h"


typedef struct { // Circular Queue
    genVec* arr;
    size_t head;     // dequeue (head++)
    size_t tail;     // enqueue   (head + size) % capacity
} __attribute__((aligned(32))) Queue;



Queue* queue_create(size_t n, size_t data_size, genVec_delete_fn del_fn);
void queue_destroy(Queue* q);
// TODO: queue_clear()

void enqueue(Queue* q, const u8* x);
void dequeue(Queue* q, u8* out);
void queue_peek(Queue* q, u8* peek);

static inline size_t queue_size(Queue* q) {
    return genVec_size(q->arr);
}

void queue_print(Queue* q, genVec_print_fn print_fn);


