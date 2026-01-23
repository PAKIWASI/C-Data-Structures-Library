#include "arena.h"


/*
align to 8 bytes
>>> 4 + 7 & ~(7)
8
align to 4 bytes
>>> 1 + 4 & ~(4)
1
*/
// Align a value to alignment boundary
#define ALIGN_UP(val, align) \
    (((val) + ((align) - 1)) & ~((align) - 1))

// align value to ARENA_DEFAULT_ALIGNMENT
#define ALIGN_UP_DEFAULT(val) \
    ALIGN_UP((val), ARENA_DEFAULT_ALIGNMENT)

// Align a pointer to alignment boundary  
// turn ptr to a u64 val to align, then turn to ptr again
#define ALIGN_PTR(ptr, align) \
    ((u8*)ALIGN_UP((u64)(ptr), (align)))

// align a pointer to ARENA_DEFAULT_ALIGNMENT
#define ALIGN_PTR_DEFAULT(ptr) \
    ALIGN_PTR((ptr), ARENA_DEFAULT_ALIGNMENT)


#define ARENA_CURR_PTR(arena) ((arena)->base + (arena)->idx)


