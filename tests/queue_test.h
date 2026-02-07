#include "Queue.h"
#include "helpers.h"
#include <stdio.h>



int queue_test_1(void)
{
    Queue* q = queue_create(100, sizeof(int), NULL, NULL, NULL);

    int a = 5;
    enqueue(q, cast(a));
    a++;
    enqueue(q, cast(a));
    a++;
    enqueue(q, cast(a));
    a++;
    enqueue(q, cast(a));
    a++;
    enqueue(q, cast(a));
    a++;
    enqueue(q, cast(a));
    a++;
    queue_print(q, int_print);putchar('\n');

    dequeue(q, NULL);
    dequeue(q, NULL);
    dequeue(q, NULL);
    queue_print(q, int_print);putchar('\n');

    queue_reset(q);
    queue_print(q, int_print);putchar('\n');

    enqueue(q, cast(a));
    queue_print(q, int_print);putchar('\n');

    queue_destroy(q);
    return 0;
}

int queue_test_2(void)
{
    


    return 0;
}
