#ifndef DA_ARENA_H
#define DA_ARENA_H

#include <thirdparty/kvec.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define ARENA_RELIABLE_MARK (1 << 0)
#define ARENA_ZERO_OUT (1 << 1)
#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define DEFAULT_CAPACITY 1024
#define MIN_FREE_SIZE 8

/* ======================= memory
           ^pos          ^size
   =======================
*/

typedef struct {
  size_t pos;
  void* mem;
} chunk;

typedef struct {
  size_t pos;
  size_t chunkid;
  size_t allocsid;
} arena_mark_t;

typedef struct {
  kvec_t(chunk) chunks;
  kvec_t(void*) allocs; // for size >= chunksize
  size_t size;
  size_t minempty;
  uint8_t flags;
} arena;

// work reliably only with ARENA_RELIABLE_MARK 
void arena_mark_reset(arena_mark_t* m, arena* ar);
arena_mark_t arena_mark(arena* ar);

arena* arena_new(size_t size, uint8_t flags);
chunk chunk_new(size_t size, uint8_t flags);
void* arena_alloc(arena* ar, size_t size);
void arena_free(arena* ar);
void arena_reset(arena* ar);

#endif
