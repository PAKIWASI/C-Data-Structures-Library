#include "Stack.h"



Stack* stack_create(u32 n, u16 data_size, genVec_delete_fn del_fn)
{
    CHECK_FATAL(data_size == 0, "data_size can't be 0");

    Stack* stk = malloc(sizeof(Stack));
    CHECK_FATAL(!stk, "stk malloc failed");

    stk->arr = genVec_init(n, data_size,NULL, NULL, del_fn);

    return stk;
}


void stack_destroy(Stack* stk)
{
    CHECK_FATAL(!stk, "stk is null");

    genVec_destroy(stk->arr);

    free(stk);
}


void stack_clear(Stack* stk)
{
    genVec_clear(stk->arr);
}

void stack_push(Stack* stk, const u8* x)
{
    CHECK_FATAL(!stk, "stk is null");

    genVec_push(stk->arr, x);
}


void stack_pop(Stack* stk, u8* popped)
{
    CHECK_FATAL(!stk, "stk is null");

    genVec_pop(stk->arr, popped);
}


void stack_peek(Stack* stk, u8* peek)
{
    CHECK_FATAL(!stk, "stk is null");
    CHECK_FATAL(!peek, "peek is null");

    genVec_get(stk->arr, genVec_size(stk->arr) - 1, peek);
}


void stack_print(Stack* stk, genVec_print_fn print_fn)
{
    CHECK_FATAL(!stk, "stk is null");
    CHECK_FATAL(!print_fn, "print_fn is null");

    genVec_print(stk->arr,print_fn);
}


