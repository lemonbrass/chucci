#ifndef DA_INTERN_H
#define DA_INTERN_H

#include <da_arena.h>
#include <thirdparty/kvec.h>
#include <da_string.h>
#include <stdint.h>

typedef string* interned_str;

typedef struct {
 kvec_t(string) entries;
 arena_t* arena;
 uint32_t seed;
 uint32_t cap;
 uint32_t len;
} InternTable;

InternTable* new_interntable();
interned_str intern(InternTable* it, string_view str);

#endif
