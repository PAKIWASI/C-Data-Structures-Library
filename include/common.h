#pragma once



// LOGGING/ERRORS

#include <stdlib.h>
#include <stdio.h>

#define WARN(fmt, ...) do {                                 \
    printf("[WARN] %s:%d:%s(): " fmt "\n",                  \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);   \
} while(0)

#define FATAL(fmt, ...) do {                                \
    fprintf(stderr, "[FATAL] %s:%d:%s(): " fmt "\n",        \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);   \
    exit(EXIT_FAILURE); \
} while (0)


#define ASSERT_WARN(cond, fmt, ...) do {        \
    if (!(cond)) {                              \
        WARN("Assertion failed: (%s): " fmt,    \
              #cond, ##__VA_ARGS__);            \
    }                                           \
} while (0)

#define ASSERT_WARN_VOID(cond, fmt, ...) do {   \
    if (!(cond)) {                              \
        WARN("Assertion failed: (%s): " fmt,    \
              #cond, ##__VA_ARGS__);            \
        return;                                 \
    }                                           \
} while (0)

#define ASSERT_WARN_NULL(cond, fmt, ...) do {   \
    if (!(cond)) {                              \
        WARN("Assertion failed: (%s): " fmt,    \
              #cond, ##__VA_ARGS__);            \
        return NULL;                            \
    }                                           \
} while (0)

#define ASSERT_FATAL(cond, fmt, ...) do {       \
    if (!(cond)) {                              \
        FATAL("Assertion failed: (%s): " fmt,   \
              #cond, ##__VA_ARGS__);            \
    }                                           \
} while (0)

#define CHECK_WARN(cond, fmt, ...) do {     \
    if ((cond)) {                           \
        WARN("Couldn't Ensure: (%s): " fmt, \
             #cond, ##__VA_ARGS__);         \
    }                                       \
} while (0)

#define CHECK_WARN_VOID(cond, fmt, ...) do {    \
    if ((cond)) {                               \
        WARN("Couldn't Ensure: (%s): " fmt,     \
              #cond, ##__VA_ARGS__);            \
        return;                                 \
    }                                           \
} while (0)

#define CHECK_WARN_RET(cond, ret, fmt, ...) do {    \
    if ((cond)) {                                   \
        WARN("Couldn't Ensure: (%s): " fmt,         \
              #cond, ##__VA_ARGS__);                \
        return ret;                                 \
    }                                               \
} while (0)

#define CHECK_FATAL(cond, fmt, ...) do {        \
    if (cond) {                                 \
        FATAL("Couldn't Ensure: (%s): " fmt,    \
             #cond, ##__VA_ARGS__);             \
    }                                           \
} while (0)


// TYPES

#include <stdint.h>

typedef uint8_t u8;
typedef uint8_t b8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


// CASTING

#define cast(x)     ((u8*)(&(x)))
#define castptr(x)  ((u8*)(x))


// COMMON SIZES

#define KB     (1 << 10)
#define MB     (1 << 20)

#define nKB(n) ((n) * KB)
#define nMB(n) ((n) * MB)


