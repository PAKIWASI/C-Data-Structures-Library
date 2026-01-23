#include "arena.h"
#include "common.h"


// align PTR to SIZE. (int -> 4 bytes, double -> 8 bytes)
#define ALIGN(ptr, size) (((ptr) + ((size) - 1)) & ~((size) - 1))

#define PTR(arena) (arena->base + arena->size)






