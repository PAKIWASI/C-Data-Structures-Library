#pragma once


#include <stdlib.h>


#define ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d:%s(): " fmt "\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define FATAL(fmt, ...) do { \
    ERROR(fmt, ##__VA_ARGS__); \
    exit(EXIT_FAILURE); \
} while (0)


