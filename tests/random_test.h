#ifndef RANDOM_TEST_H
#define RANDOM_TEST_H

#include "random.h"


int random_test_1(void)
{
    pcg32_rand_seed(1, 3);

    printf("Normal rand\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%d\n", pcg32_rand());
    }

    printf("bounded rand\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%d\n", pcg32_rand_bounded(100));
    }

    printf("\n Time Seed\n");
    pcg32_rand_seed_time();

    printf("Normal rand\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%d\n", pcg32_rand());
    }

    printf("bounded rand\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%d\n", pcg32_rand_bounded(100));
    }


    printf("\n High Precision Time Seed\n");
    pcg32_rand_seed_time_hp();

    printf("Normal rand\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%d\n", pcg32_rand());
    }

    printf("bounded rand\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%d\n", pcg32_rand_bounded(100));
    }

    return 0;
}

int random_test_2(void)
{
    pcg32_rand_seed(1, 1);

    printf("Normal rand float[0, 1]\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%f\n", pcg32_rand_float());
    }

    printf("Normal rand double[0, 1]\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%f\n", pcg32_rand_double());
    }

    printf("rand float range\n");
    for (u64 i = 0; i < 10; i++) {
        printf("%f\n", pcg32_rand_float_range(1, 3));
    }


    return 0;
}


#endif // RANDOM_TEST_H
