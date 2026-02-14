#include "fast_math.h"

#include "common.h"



float fast_sqrt(float x)
{
    if (x <= 0.0f) { return 0.0f; }
    
    // Initial guess using bit hack (speeds convergence)
    union { float f; u32 i; } conv = { .f = x };
    conv.i = 0x1fbd1df5 + (conv.i >> 1);
    float guess = conv.f;
    
    // Newton-Raphson: next = 0.5 * (guess + x/guess)
    // 3-4 iterations gives good precision
    guess = 0.5f * (guess + x / guess);
    guess = 0.5f * (guess + x / guess);
    guess = 0.5f * (guess + x / guess);
    
    return guess;
}

float fast_log(float x)
{
    if (x <= 0.0f) { return -1e10f; }  // Error value
    
    // Reduce x to range [0.5, 1.5] for better convergence
    int exp_adjust = 0;
    while (x > 1.5f) { x *= 0.5f; exp_adjust++; }
    while (x < 0.5f) { x *= 2.0f; exp_adjust--; }
    
    // Now compute ln(x) using ln(1+t) series where t = x-1
    float t = x - 1.0f;
    float t2 = t * t;
    float t3 = t2 * t;
    float t4 = t3 * t;
    float t5 = t4 * t;
    
    float result = t - (t2/2.0f) + (t3/3.0f) - (t4/4.0f) + (t5/5.0f);
    
    // Adjust for range reduction: ln(x * 2^n) = ln(x) + n*ln(2)
    result += 0.693147180559945f * (float)exp_adjust; // ln(2)

    return result;
}

float fast_sin(float x)
{
    // Reduce to [-π, π]
    const float PI = 3.14159265359f;
    const float TWO_PI = 6.28318530718f;
    
    // Normalize to [-π, π]
    while (x > PI) { x -= TWO_PI; }
    while (x < -PI) { x += TWO_PI; }
    
    float x2 = x * x;
    float x3 = x2 * x;
    float x5 = x3 * x2;
    float x7 = x5 * x2;
    
    return x - (x3/6.0f) + (x5/120.0f) - (x7/5040.0f);
}

float fast_cos(float x)
{
    // cos(x) = sin(x + π/2)
    const float HALF_PI = 1.57079632679f;
    return fast_sin(x + HALF_PI);
}


float fast_exp(float x)
{

    return 0.0f;
}

