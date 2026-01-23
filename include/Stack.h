#ifndef STACK_H
#define STACK_H

#include "gen_vector.h"



typedef struct {
    genVec* arr;
} Stack;


Stack* stack_create(u32 n, u16 data_size, genVec_delete_fn del_fn);
void stack_destroy(Stack* stk);
void stack_clear(Stack* stk);

void stack_push(Stack* stk, const u8* x);
void stack_pop(Stack* stk, u8* popped);
void stack_peek(Stack* stk, u8* peek);

static inline u32 stack_size(Stack* stk) {
    return genVec_size(stk->arr);
}

void stack_print(Stack* stk, genVec_print_fn print_fn);


#endif // STACK_H
