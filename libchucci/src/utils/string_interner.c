#include "utils/string_interner.h"
#include "utils/chucci_alloc.h"
#include "utils/string.h"
#include "utils/vec.h"
#include "utils/vmem_arena.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define is_pow_2(n) ((n & (n - 1)) == 0)
#define try_get_nth_char(str, n) ((str).len > n ? (str).cstr[n] : 0)
#define wrap_around(num, cap) (num & (cap - 1))
#define entry_sv_eq(interner, entry, sv, ch1)                                  \
  (entry->char1 == ch1 &&                                                      \
   anystr_eq(*get_interned_str(interner, entry->offset), sv))

VEC_IMPL(InternEntry, InternEntryVec, intrn_entries, MALLOC_ALLOC_INT)

StringInterner *interner_new(VMEMArena *arena) {
  StringInterner *interner = vmarena_alloc(arena, sizeof(StringInterner));
  interner->cap = DEFAULT_STRING_INTERNER_CAP;
  assert(is_pow_2(interner->cap));
  interner->entries = intrn_entries_new();
  intrn_entries_resize(&interner->entries, interner->cap, NULL);

  for (size_t i = 0; i < interner->cap; i++) {
    InternEntry *entry = intrn_entries_access_ptr(&interner->entries, i);
    entry->is_empty = 1;
  }

  interner->arena = arena;
  return interner;
}

InternedStr *get_interned_str(StringInterner *interner, uint32_t offset) {
  return (InternedStr *)(interner->arena->data + offset);
}

StringView get_interned_sv(StringInterner *interner, StringID id) {
  InternedStr *interned = get_interned_str(interner, id);
  return anystr_to_sv(*interned);
}

InternEntry *find_slot(StringView str, StringInterner *interner) {
  uint32_t hash = hash_string(str);
  size_t id = wrap_around(hash, interner->cap);
  char char1 = try_get_nth_char(str, 0);
  InternEntry *entry = intrn_entries_access_ptr(&interner->entries, id);

  if (entry->is_empty)
    return entry;
  else if (entry_sv_eq(interner, entry, str, char1))
    return entry;

  // Linear probing
  while (1) {
    entry = intrn_entries_access_ptr(&interner->entries,
                                     wrap_around(++id, interner->cap));
    if (entry->is_empty)
      return entry;
    else if (entry_sv_eq(interner, entry, str, char1)) {
      return entry;
    }
  }
}

// The default interner capacity is big enough that we wont need this
// but, by chance, if we do, I added this
void interner_resize(StringInterner *interner) {
  size_t old_cap = interner->cap;
  InternEntryVec old_entries = interner->entries;

  interner->cap *= 2;

  interner->entries = intrn_entries_new();
  intrn_entries_resize(&interner->entries, interner->cap, NULL);

  for (size_t i = 0; i < old_cap; i++) {
    InternEntry *entry = intrn_entries_access_ptr(&old_entries, i);
    if (entry->is_empty)
      continue;
    StringView str = get_interned_sv(interner, entry->offset);
    InternEntry *new_entry = find_slot(str, interner);
    *new_entry = *entry;
  }

  intrn_entries_free(&old_entries, NULL);
}

StringID populate_entry(InternEntry *entry, StringInterner *interner,
                        StringView str) {
  char char1 = try_get_nth_char(str, 0);
  InternedStr *interned =
      vmarena_alloc(interner->arena, sizeof(InternedStr) + str.len);
  interner->len++;
  entry->offset = (uint32_t)((uint8_t *)interned - interner->arena->data);
  memcpy(interned->cstr, str.cstr, str.len);
  interned->len = str.len;
  entry->char1 = char1;
  entry->is_empty = 0;
  return entry->offset;
}

StringID intern(StringView str, StringInterner *interner) {
  if (interner->len > interner->cap * DEFAULT_INTERNER_RESIZE_RATIO)
    interner_resize(interner);
  InternEntry *entry = find_slot(str, interner);
  if (entry->is_empty)
    return populate_entry(entry, interner, str);
  return entry->offset;
}

void interner_free(StringInterner *interner) {
  intrn_entries_free(&interner->entries, NULL);
}
