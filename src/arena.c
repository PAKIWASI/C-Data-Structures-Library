#include "arena.h"
#include "common.h"


// align PTR to SIZE. (int -> 4 bytes, double -> 8 bytes)
#define ALIGN(ptr, size) (((ptr) + ((size) - 1)) & ~((size) - 1))

#define PTR(arena) (arena->base + arena->size)

#define ARENA_DEFAULT_SIZE (nKB(4))




Arena* arena_create(u32 capacity)
{
    if (capacity == 0) {
        capacity = ARENA_DEFAULT_SIZE;
    }

    Arena* arena = (Arena*)malloc(capacity);
    ASSERT_FATAL(!arena, "arena malloc failed");

}

void arena_clear(Arena* arena)
{
    CHECK_FATAL(!arena, "arena is null");
    arena->size = 0;
}

void arena_release(Arena* arena)
{
    CHECK_FATAL(!arena, "arena is null");
    free(arena->base);
    free(arena);
}

void arena_push(Arena* arena, const u8* data, u32 size)
{
    CHECK_FATAL(!arena, "arena is null");
    CHECK_WARN_RET(arena->capacity - arena->size < size, , "not enough space in arena for SIZE");
    memcpy(PTR(arena), data, size);
}

void arena_pop(Arena* arena, u32 size)
{
    CHECK_FATAL(!arena, "arena is null");
    CHECK_WARN_RET(arena->size - 1 < size, , "not enough size in arena");

    arena->size -= size;
}


u32 arena_get_mark(Arena* arena)
{
    return arena->size;
}


