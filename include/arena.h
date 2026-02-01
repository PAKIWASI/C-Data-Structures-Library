#ifndef ARENA_H
#define ARENA_H

#include "common.h"


// TODO: macros to add arrays etc into arena (like in vid)


typedef struct {
    u8* base;
    u32 idx;
    u32 size;
} Arena;

 
// Tweakable settings
#define ARENA_DEFAULT_ALIGNMENT (sizeof(u64))   // 8 byte
#define ARENA_DEFAULT_SIZE (nKB(4))             // 4 KB


/*
Allocate and return a pointer to memory to the arena
with a region with the specified size. Providing a
size = 0 results in size = ARENA_DEFAULT_SIZE (user can modify)

Parameters:
  u32 size    |    The size (in bytes) of the arena
                      memory region.
Return:
  Pointer to arena on success, NULL on failure
*/
Arena* arena_create(u32 capacity);

/*
Initialize an arena object with pointers to the arena and a
pre-allocated region(base ptr), as well as the size of the provided
region. Good for using the stack instead of the heap.
The arena and the data may be stack initialized, so no arena_release.
Note that ARENA_DEFAULT_SIZE is not used.

Parameters:
  Arena* arena    |   The arena object being initialized.
  u8*    data     |   The region to be arena-fyed.
  u32    size     |   The size of the region in bytes.
*/
void arena_create_stk(Arena* arena, u8* data, u32 size);

/*
Reset the pointer to the arena region to the beginning
of the allocation. Allows reuse of the memory without
expensive frees.

Parameters:
  Arena *arena    |    The arena to be cleared.
*/
void arena_clear(Arena* arena);

/*
Free the memory allocated for the entire arena region.

Parameters:
  Arena *arena    |    The arena to be destroyed.
*/
void arena_release(Arena* arena);

/*
Return a pointer to a portion of specified size of the
specified arena's region. Nothing will restrict you
from allocating more memory than you specified, so be
mindful of your memory (as you should anyways) or you
will get some hard-to-track bugs. By default, memory is
aligned by alignof(size_t), but you can change this by
#defining ARENA_DEFAULT_ALIGNMENT before #include'ing
arena.h. Providing a size of zero results in a failure.

Parameters:
  Arena* arena    |    The arena of which the pointer
                       from the region will be
                       distributed
  u32 size        |    The size (in bytes) of
                       allocated memory planned to be
                       used.
Return:
  Pointer to arena region segment on success, NULL on
  failure.
*/
u8* arena_alloc(Arena* arena, u32 size);

/*
Same as arena_alloc, except you can specify a memory
alignment for allocations.

Return a pointer to a portion of specified size of the
specified arena's region. Nothing will restrict you
from allocating more memory than you specified, so be
mindful of your memory (as you should anyways) or you
will get some hard-to-track bugs. Providing a size of
zero results in a failure.

Parameters:
  Arena* arena              |    The arena of which the pointer
                                 from the region will be
                                 distributed
  u32 size                  |    The size (in bytes) of
                                 allocated memory planned to be
                                 used.
  u16 alignment             |    Alignment (in bytes) for each
                                 memory allocation.
Return:
  Pointer to arena region segment on success, NULL on
  failure.
*/
u8* arena_alloc_aligned(Arena* arena, u32 size, u16 alignment);


/*
Get the value of index at the current state of arena
This can be used to later clear upto that point using arena_clear_mark

Parameters:
  Arena* arena          |   The arena whose idx will be returned

Return:
  The current value of idx variable
*/
u32 arena_get_mark(Arena* arena);

/*
Clear the arena from current index back to mark

Parameters:
  Arena* arena          |   The arena you want to clear using it's mark
  u32    mark           |   The mark previosly obtained by arena_get_mark 
*/
void arena_clear_mark(Arena* arena, u32 mark);

// Get used capacity
static inline u32 arena_used(Arena* arena)
{
    CHECK_FATAL(!arena, "arena is null");
    return arena->idx;
}

// Get remaining capacity
static inline u32 arena_remaining(Arena* arena)
{
    CHECK_FATAL(!arena, "arena is null");
    return arena->size - arena->idx;
}


// USEFULL MACROS


// typed allocation
#define ARENA_ALLOC(arena, T) \
    ((T*)arena_alloc((arena), sizeof(T)))

#define ARENA_ALLOC_N(arena, T, n) \
    ((T*)arena_alloc((arena), sizeof(T) * (n)))

// common for structs
#define ARENA_ALLOC_ZERO(arena, T) \
    ((T*)memset(ARENA_ALLOC(arena, T), 0, sizeof(T)))

#define ARENA_ALLOC_ZERO_N(arena, T, n) \
    ((T*)memset(ARENA_ALLOC_N(arena, T, n), 0, sizeof(T) * (n)))

// Scratch Arena

#define ARENA_SCRATCH(arena) \
    for (u32 _mark = arena_get_mark(arena); _mark != (u32)-1; \
         arena_clear_mark(arena, _mark), _mark = (u32)-1)
/* USAGE:
ARENA_SCRATCH(arena) {
    char* tmp = ARENA_ALLOC_N(arena, char, 256);
    // temp work
} // auto rollback
*/



#endif // ARENA_H
