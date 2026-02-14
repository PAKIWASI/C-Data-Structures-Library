#ifndef FAST_MATH_H
#define FAST_MATH_H


/*
    Implement costy math functions using Numerical Methods
    This is faster than math lib, but with (way) less precision
    Use when you dont care about very precise values
    Originally written to be used for rng gaussian 
*/



// Fast sqrt using Newton-Raphson iteration (Thank u Professor Abrar)
float fast_sqrt(float x);

// Natural log using series expansion: ln(1+x) = x - x²/2 + x³/3 - x⁴/4 + ...
float fast_log(float x);

// Fast sine using Taylor series: sin(x) = x - x³/6 + x⁵/120 - ...
float fast_sin(float x);

// cos(x) = sin(x + π/2)
float fast_cos(float x);

// TODO: e^x
float fast_exp(float x);


// TODO: higher precision versions ? 
// by increasing iteration count / Taylor series terms


#endif // FAST_MATH_H
