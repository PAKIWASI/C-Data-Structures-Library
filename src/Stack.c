#include "Stack.h"



Stack* stack_create(u64 n, u32 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn, genVec_delete_fn del_fn)
{
    return genVec_init(n, data_size, copy_fn, move_fn, del_fn);
}

Stack* stack_create_val(u64 n, const u8* val, u32 data_size, genVec_copy_fn copy_fn, genVec_move_fn move_fn,
                        genVec_delete_fn del_fn)
{
    return genVec_init_val(n, val, data_size, copy_fn, move_fn, del_fn);
}


void stack_destroy(Stack* stk)
{
    genVec_destroy(stk);
}

void stack_clear(Stack* stk)
{
    genVec_clear(stk);
}

void stack_reset(Stack* stk)
{
    genVec_reset(stk);
}

void stack_push(Stack* stk, const u8* x)
{
    genVec_push(stk, x);
}

void stack_push_move(Stack* stk, u8** x)
{
    genVec_push_move(stk, x);
}

void stack_pop(Stack* stk, u8* popped)
{
    genVec_pop(stk, popped);
}

void stack_peek(Stack* stk, u8* peek)
{
    genVec_get(stk, genVec_size(stk) - 1, peek);
}

const u8* stack_peek_ptr(Stack* stk)
{
    return genVec_get_ptr(stk, genVec_size(stk) - 1);
}

void stack_print(Stack* stk, genVec_print_fn print_fn)
{
    genVec_print(stk, print_fn);
}


