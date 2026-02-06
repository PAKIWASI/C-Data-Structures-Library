



#include "Stack.h"
int stack_test_1(void)
{
    Stack* stk = stack_create(10, sizeof(int), NULL, NULL, NULL);


    stack_destroy(stk);
    return 0;
}
