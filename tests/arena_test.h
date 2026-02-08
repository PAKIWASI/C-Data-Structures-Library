#ifndef	ARENA_TEST_H
#define ARENA_TEST_H


#include "arena.h"
#include "gen_vector.h"
#include <stdio.h>


int arena_test_1(void)
{
    Arena* arena = arena_create(0); 

    u32 mark = arena_get_mark(arena);
    for (int i = 0; i < 100; i++) {
        long a = 1;
        long b = 2;
        long c = 3;
        long d = 4; 
        long* ptr = (long*)arena_alloc(arena, sizeof(long) * 4);
        ptr[0] = a;
        ptr[1] = b;
        ptr[2] = c;
        ptr[3] = d;
    }
    arena_clear_mark(arena, mark);

    print_hex(arena->base, sizeof(long) * 6, 8);

    u32 mark2 = arena_get_mark(arena);
    for (int i = 0; i < 100; i++) {
        long a = 100;
        long b = 200;
        long c = 300;
        long d = 400; 
        long* ptr = (long*)arena_alloc(arena, sizeof(long) * 4);
        ptr[0] = a;
        ptr[1] = b;
        ptr[2] = c;
        ptr[3] = d;
    }
    arena_clear_mark(arena, mark2);

    print_hex(arena->base, sizeof(long) * 6, 8);

    arena_release(arena);
    return 0;
}

int arena_test_2(void)
{
    Arena arena;
    int buff[nKB(1)];
    arena_create_arr_stk(&arena, (u8*)buff, nKB(1));


    float* b = (float*)arena_alloc_aligned(&arena, sizeof(float), sizeof(float));
    b[0] = 2.1F;

    double* a = (double*)arena_alloc(&arena, sizeof(double));
    a[0] = 1.1111;

    print_hex(arena.base, 16, 4);

    return 0;
}

int arena_test_3(void)
{
    Arena* arena = arena_create(0);

    print_hex(castptr(arena), sizeof(*arena), 8);
    putchar('\n');

    ARENA_SCRATCH(temp, arena) {
        u8* a = arena_alloc(arena, nKB(1));

        print_hex(castptr(arena), sizeof(*arena), 8);
        putchar('\n');
    }

    print_hex(castptr(arena), sizeof(*arena), 8);
    putchar('\n');

    arena_scratch sc = arena_scratch_begin(arena);

    u8* a = arena_alloc(arena, nKB(2));

    print_hex(castptr(arena), sizeof(*arena), 8);
    putchar('\n');

    arena_scratch_end(&sc);

    print_hex(castptr(arena), sizeof(*arena), 8);
    putchar('\n');

    arena_release(arena);
    return 0;
}


#endif // ARENA_TEST_H
