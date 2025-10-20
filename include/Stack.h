#pragma once

#include "gen_vector.h"



typedef struct {
    genVec* arr;
} Stack;


Stack* stack_create(size_t n, size_t data_size, genVec_delete_fn del_fn);
void stack_destroy(Stack* stk);
void stack_clear(Stack* stk);

void stack_push(Stack* stk, const u8* x);
void stack_pop(Stack* stk, u8* popped);
void stack_peek(Stack* stk, u8* peek);

static inline size_t stack_size(Stack* stk) {
    return genVec_size(stk->arr);
}

void stack_print(Stack* stk, genVec_print_fn print_fn);
