#include "Queue.h"
#include "gen_vector.h"

#include <stdio.h>
#include <stdlib.h>



static inline void tail_update(Queue* q) {
    q->tail = (q->head + genVec_size(q->arr)) % q->arr->capacity; 
}

Queue* queue_create(size_t n, size_t data_size, genVec_delete_fn del_fn)
{
    Queue* q = malloc(sizeof(Queue));
    if (!q) {
        printf("queue create: malloc failed\n");
        return NULL;
    }

    q->arr = genVec_init(n, data_size, del_fn);
    if (!q->arr) {
        printf("queue create: vec init failed\n");
        free(q);
        return NULL;
    }
    q->head = 0;   // (head++)
    q->tail = 0;   // (head + size) % capacity 
    
    return q;
}


void queue_destroy(Queue* q)
{
    if (!q) { return; }

    genVec_destroy(q->arr);

    free(q);
}


void enqueue(Queue* q, const u8* x)
{
    if (!q || ! x) {
        printf("enqueue: parameters null\n");
        return;
    }

    genVec_insert(q->arr, q->tail, x);
    tail_update(q);

    // TODO: growth? 
}


void dequeue(Queue* q, u8* out)
{
    if (!q) {
        printf("dequeue: parameters null\n");
        return;
    }

    if (out) {
        genVec_get(q->arr, q->head, out);
    }

    q->head++;
}


void queue_peek(Queue* q, u8* peek)
{
    if (!q || !peek) {
        printf("queue peek: parameters null\n");
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

    size_t h = q->head;

    printf("[ ");
    if (h < q->arr->size) {
        do {
            u8* out = malloc(q->arr->data_size);
            genVec_get(q->arr, h, out);
            print_fn(out);
            printf(" ");
            free(out);
            h = (h + 1) % q->arr->capacity;
        }
        while (h != q->tail);
    }

    printf("]\n");
}
