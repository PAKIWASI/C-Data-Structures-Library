#include "Queue.h"
#include "gen_vector.h"

#include <stdio.h>
#include <stdlib.h>


#define QUEUE_GROWTH 1.5


static void tail_update(Queue* q);
static void queue_grow(Queue* q);


Queue* queue_create(u32 n, u32 data_size, genVec_delete_fn del_fn)
{
    if (!n || !data_size) {
        printf("queue create : invalid parameters\n");
    }
    Queue* q = malloc(sizeof(Queue));
    if (!q) {
        printf("queue create: malloc failed\n");
        return NULL;
    }

    u8* dummy = malloc(sizeof(data_size));
    q->arr = genVec_init_val(n, dummy, data_size, del_fn);
    free(dummy);
    if (!q->arr) {
        printf("queue create: vec init failed\n");
        free(q);
        return NULL;
    }
    q->head = 0;   // (head + 1) % capacity
    q->tail = 0;   // (head + size) % capacity 
    q->size = 0;
    
    return q;
}


void queue_destroy(Queue* q)
{
    if (!q) { return; }

    if (q->arr) {
        genVec_destroy(q->arr);
    }

    free(q);
}

void queue_clear(Queue* q)
{
    if (!q) {
        printf("queue clear: queue null\n");
        return;
    }

    q->size = 0;
    q->head = 0;
    q->tail = 0;
}


void enqueue(Queue* q, const u8* x)
{
    if (!q || !x) {
        printf("enqueue: parameters null\n");
        return;
    }

    genVec_replace(q->arr, q->tail, x);
    q->size++;
    tail_update(q);

    if (q->size == q->arr->capacity) {
        queue_grow(q);
    }
}

void dequeue(Queue* q, u8* out)
{
    if (!q) {
        printf("dequeue: queue null\n");
        return;
    }
    
    if (q->size == 0) { 
        printf("dequeue: queue is empty\n");
        return;
    }

    if (out) {
        genVec_get(q->arr, q->head, out);
    }

    u8* dummy = malloc(sizeof(q->arr->data_size));
    genVec_replace(q->arr, q->head, dummy);
    free(dummy);
    q->head = (q->head + 1) % q->arr->capacity;
    q->size--;
    // TODO:  no shrink for now

}

void queue_peek(Queue* q, u8* peek)
{
    if (!q || !peek) {
        printf("peek: parameters null\n");
        return;
    }
    if (q->size == 0) {
        printf("peek: queue empty\n");
        return;
    }

    genVec_get(q->arr, q->head, peek);
}

void queue_print(Queue* q, genVec_print_fn print_fn)
{
    if (!q || !print_fn) {
        printf("queue print: parameters null\n");
        return;
    } 

    u32 h = q->head;

    printf("[ ");
    if (h < q->arr->size && q->size != 0) {
        do {
            const u8* out = genVec_get_ptr(q->arr, h);
            print_fn(out);
            printf(" ");
            h = (h + 1) % q->arr->capacity;
        }
        while (h != q->tail);
    }

    printf("]\n");
}

static void tail_update(Queue* q) {
    q->tail = (q->head + q->size) % q->arr->capacity; 
}

static void queue_grow(Queue* q) {
    u32 old_cap = q->arr->capacity;
    u32 new_cap = (u32)((double)old_cap * QUEUE_GROWTH);
    
    genVec_reserve(q->arr, new_cap);

    u8* dummy = malloc(q->arr->data_size); // Expand the vector's size to match capacity
    for (u32 i = old_cap; i < new_cap; i++) {
        genVec_push(q->arr, dummy); 
    }
    free(dummy);   

    // Move wrapped elements from [0, head) to [old_cap, old_cap + head)
    for (u32 i = 0; i < q->head; i++) {
        genVec_replace(q->arr, old_cap + i, genVec_get_ptr(q->arr, i));
    }
    
    // Update tail to point to the new end of the queue
    tail_update(q);
}


