#pragma once



// LOGGING/ERRORS

#include <stdlib.h>
#include <stdio.h>

#define WARN(fmt, ...) \
    printf("[WARN] %s:%d:%s(): " fmt "\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__)


#define ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d:%s(): " fmt "\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define FATAL(fmt, ...) do { \
    ERROR(fmt, ##__VA_ARGS__); \
    exit(EXIT_FAILURE); \
} while (0)


#define ASSERT(cond, fmt, ...) do {            \
    if (!(cond)) {                             \
        ERROR("Assertion failed: (%s): " fmt,  \
              #cond, ##__VA_ARGS__);           \
    }                                          \
} while (0)

#define ASSERT_FATAL(cond, fmt, ...) do {      \
    if (!(cond)) {                             \
        FATAL("Assertion failed: (%s): " fmt,  \
              #cond, ##__VA_ARGS__);           \
        abort();                               \
    }                                          \
} while (0)


// TYPES

#include <stdint.h>

typedef uint8_t u8;
typedef uint8_t b8;
typedef uint16_t u16;
typedef uint32_t u32;



// COMMON SIZES

#define KB     (1 << 10)
#define MB     (1 << 20)

#define nKB(n) ((n) * KB)
#define nMB(n) ((n) * MB)


