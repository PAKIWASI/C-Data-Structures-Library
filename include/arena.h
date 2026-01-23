#pragma once

#include "common.h"




typedef struct {
    u8* base;
    u32 capacity;
    u32 size;
} Arena;



Arena* arena_create(u32 capacity);
void* arena_alloc(Arena* arena, u32 size);
void arena_clear(Arena* arena);
void arena_release(Arena* arena);

void arena_push(Arena* arena, const u8* data, u32 size);
void arena_pop(Arena* arena, u32 size);

u32 arena_get_mark(Arena* arena);
void arena_clear_mark(Arena* arena, u32 mark);
