#include "Stack.h"
#include "helpers.h"


int stack_test_1(void)
{
    Stack* stk = stack_create(10, sizeof(int), NULL, NULL, NULL);

    int a = 5;
    stack_push(stk, cast(a));
    stack_push(stk, cast(a));
    stack_push(stk, cast(a));
    stack_push(stk, cast(a));
    stack_push(stk, cast(a));
    stack_push(stk, cast(a));
    stack_push(stk, cast(a));
    stack_push(stk, cast(a));
    stack_push(stk, cast(a));

    printf("%d\n", *(int*)stack_peek_ptr(stk));

    stack_print(stk, int_print);
    putchar('\n');

    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);
    stack_pop(stk, NULL);

    stack_print(stk, int_print);
    putchar('\n');

    printf("%d\n", stack_empty(stk));

    stack_destroy(stk);
    return 0;

}
