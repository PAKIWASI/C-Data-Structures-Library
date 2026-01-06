#include "Stack.h"
#include "common.h"
#include "gen_vector.h"

#include <stdio.h>



Stack* stack_create(u32 n, u32 data_size, genVec_delete_fn del_fn)
{
    if (!data_size) {
        ERROR("invalid parameters");
        return NULL;
    }
    Stack* stk = malloc(sizeof(Stack));
    if (!stk) {
        ERROR("malloc failed");
        return NULL;
    }

    stk->arr = genVec_init(n, data_size, del_fn);
    if (!stk->arr) {
        ERROR("vec init failed");
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
        ERROR("parameters null");
        return;
    } 

    genVec_push(stk->arr, x);
}


void stack_pop(Stack* stk, u8* popped)
{
    if (!stk) {
        ERROR("stk null");
    }

    genVec_pop(stk->arr, popped);
}


void stack_peek(Stack* stk, u8* peek)
{
    if (!stk || !peek) {
        ERROR("parameters null");
        return;
    } 

    genVec_get(stk->arr, genVec_size(stk->arr) - 1, peek);
}


void stack_print(Stack* stk, genVec_print_fn print_fn)
{
    if (!stk || !print_fn) {
        ERROR("parameters null");
        return;
    }

    genVec_print(stk->arr,print_fn);
}


