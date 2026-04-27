#include <da_arena.h>

void arena_mark_reset(arena_mark_t* m, arena* ar) {
  for (size_t i = m->chunkid + 1; i < kv_size(ar->chunks); i++) {
    kv_A(ar->chunks, i).pos = 0;
  }
  for (size_t i = m->allocsid + 1; i < kv_size(ar->allocs); i++) {
    free(kv_pop(ar->allocs));
  }
  ar->minempty = m->chunkid;
  kv_A(ar->chunks, m->chunkid).pos = m->pos;
}

chunk chunk_new(size_t size, uint8_t flags) {
  chunk ch;
  ch.pos = 0;
  ch.mem = malloc(size);
  assert(ch.mem && "Arena chunk allocation failed. malloc() returned NULL");
  if (flags & ARENA_ZERO_OUT) {
    memset(ch.mem, 0, size);
  }
  return ch;
}

arena* arena_new(size_t size, uint8_t flags) {
  arena* ar = malloc(sizeof(arena));
  kv_init(ar->chunks);
  kv_init(ar->allocs);
  ar->flags = flags;
  ar->size = size;
  ar->minempty = 0;
  kv_push(chunk, ar->chunks, chunk_new(size, flags));
  
  return ar;
}

arena_mark_t arena_mark(arena* ar) {
  arena_mark_t m;
  chunk* ch = &kv_A(ar->chunks, kv_size(ar->chunks)-1);
  m.pos = ch->pos;
  m.chunkid = kv_size(ar->chunks) - 1;
  m.allocsid = kv_size(ar->allocs);
  return m;
}

chunk* get_arena_chunk(arena* ar, size_t size) {
  if (!(ar->flags & ARENA_RELIABLE_MARK)) {
    size_t id = ar->minempty;
    while (1) {
      if (kv_size(ar->chunks) <= id) {
        kv_push(chunk, ar->chunks, chunk_new(ar->size, ar->flags));
      }

      chunk* ch = &kv_A(ar->chunks, id);
      size_t free_space = ar->size - ch->pos;

      if (free_space >= size)
        return ch;

      if (free_space < MIN_FREE_SIZE)
        ar->minempty = id + 1;

      id++;
    }
  }
  else {
    chunk* ch = &kv_A(ar->chunks, kv_size(ar->chunks)-1);
    if (ar->size - ch->pos >= size) {
      return ch;
    } else {
      kv_push(chunk, ar->chunks, chunk_new(ar->size, ar->flags));
      return &kv_A(ar->chunks, kv_size(ar->chunks)-1);
    }
  }
}

void* arena_alloc(arena* ar, size_t size) {
  size_t a_size = ALIGN_UP(size, alignof(max_align_t));
  if (a_size >= ar->size) {
    void* mem = malloc(size);
    kv_push(void*, ar->allocs, mem);
    return mem;
  }
  chunk* ch = get_arena_chunk(ar, a_size);
  void* result = (char*)ch->mem + ch->pos;
  ch->pos += a_size;
  return result;
}


void chunk_free(chunk* ch) {
  free(ch->mem);
  ch->mem = NULL;
  ch->pos = 0;
}

void arena_free(arena* ar) {
  for (size_t i = 0; i < kv_size(ar->chunks); i++) {
    chunk_free(&kv_A(ar->chunks, i));
  }
  for (size_t i = 0; i < kv_size(ar->allocs); i++) {
    free(kv_A(ar->allocs, i));
  }
  kv_destroy(ar->chunks);
  free(ar);
}

void arena_reset(arena* ar) {
  for (size_t i = 0; i < kv_size(ar->chunks); i++) {
    chunk* ch = &kv_A(ar->chunks, i);
    ch->pos = 0;
  }
  for (size_t i = 0; i < kv_size(ar->allocs); i++) {
    free(kv_A(ar->allocs, i));
  }
  ar->minempty = 0;
}


