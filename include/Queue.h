#pragma once

#include "gen_vector.h"


typedef struct { // Circular Queue
    genVec* arr;
    size_t head;     // dequeue (head + 1) % capacity
    size_t tail;     // enqueue   (head + size) % capacity
    size_t size;
} __attribute__((aligned(32))) Queue;


Queue* queue_create(size_t n, size_t data_size, genVec_delete_fn del_fn);
void queue_destroy(Queue* q);
void queue_clear(Queue* q);

void enqueue(Queue* q, const u8* x);
void dequeue(Queue* q, u8* out);
void queue_peek(Queue* q, u8* peek);

static inline size_t queue_size(Queue* q) {
    return q->size;
}

static inline u8 queue_empty(Queue* q) {
    return q->size == 0;
}

void queue_print(Queue* q, genVec_print_fn print_fn);


