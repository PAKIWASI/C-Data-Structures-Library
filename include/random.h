#ifndef RANDOM_H
#define RANDOM_H

#include "common.h"

/*
    This Implimentation is Based on:
    *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
    Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)
    And Also:
    https://github.com/imneme/pcg-c-basic
*/

/* PCG
    A simple Linear Congruential Generator (LCG) does:
        next_state = (xplier * curr_state + increment) % mod
    This is pretty weak and predictable
    In 3D, it forms 2D "layers"
    
    PCG takes this LCG and scrambles it using permutation function (hash fn)
    It halfes the output bits - 64 bits of state output a 32 bit random number

    Key Idea: Use some bits to decide how to scramble the other bits
        You have 64 bits, you use top few bits as "control code"
        You use that code to decide how to shuffle/rotate/shift the remaining
*/


// Private Internal RNG State
typedef struct {
    u64 state; // RNG State - All possible values
    u64 inc;   // MUST be ODD - determines which sequence of random numbers this generator produces
               // sequence selection constant (a.k.a. stream id)

} pcg32_random_t; // get 32 bit random number from 64 bit state, increment


// Default initializer with pre-chosen values for state and increment.
#define PCG32_INITIALIZER {0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL}



void pcg32_rand_seed(u64 seed, u64 seq);


void pcg32_rand_seed_time(void);
void pcg32_rand_seed_time_hp(void);


u32 pcg32_rand(void);


u32 pcg32_rand_bounded(u32 bound);


float pcg32_rand_float(void);


double pcg32_rand_double(void);


float pcg32_rand_float_range(float min, float max);


double pcg32_rand_double_range(double min, double max);



// TODO: add gaussian distribution


#endif // RANDOM_H
