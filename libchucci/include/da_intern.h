#ifndef DA_INTERN_H
#define DA_INTERN_H

#include <da_arena.h>
#include <stdbool.h>
#include <thirdparty/kvec.h>
#include <da_string.h>
#include <stdint.h>

#ifndef INTERN_LOAD_FACTOR
#define INTERN_LOAD_FACTOR 0.8
#endif

#define interned_eq(str1, str2) ((str1).cstr == (str2).cstr)

typedef uint32_t hash_t;
typedef struct {
  char* cstr;
  uint32_t len;
} interned_str;

typedef struct {
 string str;
 hash_t h;
} InternEntry;

typedef struct {
 kvec_t(InternEntry) entries;
 arena_t* arena;
 uint32_t seed;
 uint32_t cap;
 uint32_t len;
} InternTable;

InternTable* new_interntable();
interned_str intern(InternTable* table, string_view str);

string_view interned_to_sv(interned_str str);
void free_interntable(InternTable** table);

#endif
