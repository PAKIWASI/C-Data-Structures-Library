#include "arena.h"


// align PTR to SIZE. (int -> 4 bytes, double -> 8 bytes)
#define ALIGN(ptr, size) ((ptr) + ((size) - 1) & ~((size) - 1))


#define ARENA_DEFAULT_SIZE (nKB(4))




Arena* arena_create(u32 capacity)
{
    if (capacity == 0) {
        capacity = ARENA_DEFAULT_SIZE;
    }

    Arena* arena = (Arena*)malloc(capacity);
    if (!arena) {
        ERROR("arena malloc failed");
    }
}
