#ifndef __CHUNKED_ARENA_H
#define __CHUNKED_ARENA_H

#include "utils/vec.h"
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_ARENA_CHUNK_CAP 1024*4

typedef struct ArenaChunk {
  uint8_t *data;
  size_t cap;
} ArenaChunk;

VEC_DEF_WITH_FIELDS(ArenaChunk, ChunkedArena, _chunks, size_t pos;);

typedef struct ArenaMark {
  size_t len, pos;
} ChunkedArenaMark;

ChunkedArena chnk_arena_new();
void *chnk_arena_alloc(ChunkedArena *arena, size_t size);
void chnk_arena_reset(ChunkedArena *arena);
void chnk_arena_free(ChunkedArena *arena);
void *chnk_arena_realloc(ChunkedArena *arena, void *ptr, size_t old_size, size_t new_size);
void *chnk_arena_calloc(ChunkedArena *arena, size_t size);
void chnk_arena_mark_reset(ChunkedArena* arena, ChunkedArenaMark mark);
ChunkedArenaMark chnk_arena_mark(ChunkedArena *arena);



#endif
