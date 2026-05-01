#define CV_ALLOC(size) calloc(size)

#include <da_string.h>
#include <thirdparty/kvec.h>
#include <da_arena.h>
#include <da_intern.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define INTERN_LOAD_FACTOR 0.8

typedef uint32_t hash_t;

hash_t hash_str(const char *data, size_t len, uint32_t seed) {
  uint32_t h = seed ^ (uint32_t)len;

  const uint32_t *blocks = (const uint32_t *)data;
  size_t nblocks = len / 4;

  for (size_t i = 0; i < nblocks; i++) {
    uint32_t k = blocks[i];

    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;

    h ^= k;
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
  }

  const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
  uint32_t k = 0;

  switch (len & 3) {
    case 3: k ^= tail[2] << 16;
    case 2: k ^= tail[1] << 8;
    case 1: k ^= tail[0];
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
  }

  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

InternTable* new_interntable() {
  InternTable* table = malloc(sizeof(InternTable));
  table->cap = 1024;
  table->arena = new_arena(1024, ARENA_RELIABLE_MARK & ARENA_ZERO_OUT);
  kv_init(table->entries);
  kv_resize(string, table->entries, 1024);
  memset(table->entries.a, 0, sizeof(string)*table->entries.m);
  table->seed = time(NULL);
  return table;
}

interned_str intern(InternTable* table, string_view str) {
  assert((float)table->len/(float)table->cap < INTERN_LOAD_FACTOR);
  hash_t h = hash_str(str.cstr, str.len, table->seed);
  for (size_t idx = h % table->cap; ; idx = (idx + 1) % table->cap) {
    string* entry = &kv_A(table->entries, idx);
    if (entry->cstr == NULL) {
      entry->cstr = arena_alloc(table->arena, str.len);
      entry->len = str.len;
      memcpy((void*)entry->cstr, str.cstr, str.len);
      table->len++;
      // printf("New entry: %.*s\n", (int)entry->len, entry->cstr);
      return entry;
    }
    else if (s_eq(str, *entry)) {
      // printf("Old entry: %.*s\n", (int)entry->len, entry->cstr);
      return entry;
    }
  }
}


