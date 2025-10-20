#include "Stack.h"
#include "gen_vector.h"

#include <stdio.h>
#include <stdlib.h>



Stack* stack_create(size_t n, size_t data_size, genVec_delete_fn del_fn)
{
    Stack* stk = malloc(sizeof(Stack));
    if (!stk) {
        printf("stk create: malloc failed\n");
        return NULL;
    }

    stk->arr = genVec_init(n, data_size, del_fn);
    if (!stk->arr) {
        printf("stk create: vec init failed\n");
        free(stk);
        return NULL;
    }

    return stk;
}


void stack_destroy(Stack* stk)
{
    if (!stk) { return; }

    genVec_destroy(stk->arr);

    free(stk);
}


void stack_clear(Stack* stk)
{
    genVec_clear(stk->arr);
}

void stack_push(Stack* stk, const u8* x)
{
    if (!stk || !x) {
        printf("stk push: parameters null\n");
        return;
    } 

    genVec_push(stk->arr, x);
}


void stack_pop(Stack* stk, u8* popped)
{
    if (!stk) {
        printf("stk null\n");
    }

    genVec_pop(stk->arr, popped);
}


void stack_peek(Stack* stk, u8* peek)
{
    if (!stk || !peek) {
        printf("parameters null\n");
        return;
    } 

    genVec_get(stk->arr, genVec_size(stk->arr) - 1, peek);
}


void stack_print(Stack* stk, genVec_print_fn print_fn)
{
    if (!stk || !print_fn) {
        printf("stk print: parameters null\n");
        return;
    }

    genVec_print(stk->arr,print_fn);
}
