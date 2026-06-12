#ifndef __STRING_INTERNER_H
#define __STRING_INTERNER_H

#include "utils/smallvec.h"
#include "utils/string.h"
#include "utils/vec.h"
#include "utils/vmem_arena.h"
#include <stdint.h>

#define DEFAULT_INTERNER_RESIZE_RATIO 0.8
#define DEFAULT_STRING_INTERNER_CAP 1024
typedef uint32_t StringID;
SMALLVEC_DEF(StringID, StringIDVec, stridvec, VMEM_ARENA_ALLOC_INT)


typedef struct InternedStr {
  uint16_t len;
  char cstr[];
} InternedStr;

typedef struct InternEntry {
  // As we assume max size needed by arena will be less than 1 MiB, that is
  // 1024*1024 = 2^20. I added another 3 bits (dunno what to do with them)
  // Offset to InternedStr in arena
  uint32_t offset : 23;
  // first char of string for faster comparisons, this space would be wasted
  // otherwise
  uint32_t char1 : 8;
  uint32_t is_empty : 1;
} InternEntry;

VEC_DEF(InternEntry, InternEntryVec, intrn_entries, VMEM_ARENA_ALLOC_INT)

typedef struct StringInterner {
  InternEntryVec entries;
  size_t cap; // power of 2
  size_t len;
  VMEMArena *arena;
} StringInterner;

StringInterner* interner_new(VMEMArena *arena);
StringID intern(StringView str, StringInterner *interner);
StringView get_interned_sv(StringInterner *interner, StringID id);
void interner_free(StringInterner *interner);

#endif
