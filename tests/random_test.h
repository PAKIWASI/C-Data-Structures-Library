#ifndef RANDOM_TEST_H
#define RANDOM_TEST_H

#include "random.h"


int random_test_1(void)
{
    pcg32_rand_seed(1, 3);

    for (u64 i = 0; i < 10; i++) {
        printf("%d\n", pcg32_rand());
    }

    return 0;
}


#endif // RANDOM_TEST_H
