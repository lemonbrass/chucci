#include "utils/chunked_arena.h"
#include "utils/chucci_alloc.h"
#include "utils/vec.h"
#include <stdlib.h>
#include <string.h>

VEC_IMPL(ArenaChunk, ChunkedArena, _chunks, MALLOC_ALLOC_INT);
#define ALIGN_UP(n, a) (((n) + (a) - 1) & ~((a) - 1))
#define DEFAULT_ALIGNMENT 8

ChunkedArena chnk_arena_new() { return (ChunkedArena){0}; }

ArenaChunk chunk_new(size_t cap) {
  ArenaChunk chunk = {0};
  chunk.data = malloc(cap);
  chunk.cap = cap;
  return chunk;
}

void *chnk_arena_calloc(ChunkedArena *arena, size_t size) {
  void *ptr = chnk_arena_alloc(arena, size);
  memset(ptr, 0, size);
  return ptr;
}

void *chnk_arena_realloc(ChunkedArena *arena, void *ptr, size_t old_size,
                         size_t new_size) {
  ArenaChunk top_chunk = _chunks_top(arena);
  if (new_size <= old_size)
    return ptr;
  else if (ptr + old_size == top_chunk.data + arena->pos &&
           top_chunk.cap - arena->pos >= new_size - old_size) {
    arena->pos += new_size - old_size;
    return ptr;
  } else {
    void *new_ptr = chnk_arena_alloc(arena, new_size);
    memcpy(new_ptr, ptr, old_size);
    return new_ptr;
  }
}

void *chnk_arena_alloc(ChunkedArena *arena, size_t size) {
  if (size > DEFAULT_ARENA_CHUNK_CAP) {
    _chunks_push(arena, chunk_new(size), NULL);
    arena->pos = size;
    return _chunks_top(arena).data;
  }
  if (_chunks_len(arena) == 0) {
    _chunks_push(arena, chunk_new(DEFAULT_ARENA_CHUNK_CAP), NULL);
    arena->pos = 0;
  }
  ArenaChunk *top_chunk = _chunks_top_ptr(arena);
  arena->pos = ALIGN_UP(arena->pos, DEFAULT_ALIGNMENT);

  if (top_chunk->cap - arena->pos >= size) {
    arena->pos += size;
    return top_chunk->data + arena->pos - size;
  } else {
    _chunks_push(arena, chunk_new(DEFAULT_ARENA_CHUNK_CAP), NULL);
    top_chunk = _chunks_top_ptr(arena);
    arena->pos = 0;
    arena->pos += size;
    return top_chunk->data + arena->pos - size;
  }
}

void chunk_free(ArenaChunk chunk) { free(chunk.data); }

// TODO: Do something about these frees
void chnk_arena_mark_reset(ChunkedArena *arena, ChunkedArenaMark mark) {
  while (_chunks_len(arena) > mark.len)
    chunk_free(_chunks_pop(arena));
  arena->pos = mark.pos;
  arena->len = mark.len;
}

void chnk_arena_reset(ChunkedArena *arena) {
  arena->pos = 0;
  _chunks_reset(arena);
}

void chnk_arena_free(ChunkedArena *arena) {
  while (_chunks_len(arena) > 0)
    chunk_free(_chunks_pop(arena));
  _chunks_free(arena, NULL);
  *arena = (ChunkedArena){0};
}

ChunkedArenaMark chnk_arena_mark(ChunkedArena *arena) {
  return (ChunkedArenaMark){.len = _chunks_len(arena), .pos = arena->pos};
}
