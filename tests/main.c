#include "utils/string.h"
#include "utils/string_interner.h"
#include "utils/vmem_arena.h"
#include <assert.h>
#include <stdio.h>

int main() {
  VMEMArena vmarena = vmarena_new();
  StringInterner baba = interner_new(&vmarena);

  StringID a = intern(cstr_to_anystr("baba", StringView), &baba);
  StringID b = intern(cstr_to_anystr("baba", StringView), &baba);
  StringID c = intern(cstr_to_anystr("baba", StringView), &baba);
  StringID d = intern(cstr_to_anystr("baba", StringView), &baba);

  printf("%d, %d, %d, %d\n", a, b, c, d);
  assert(
      str_eq(cstr_to_anystr("a", StringView), cstr_to_anystr("a", StringView)));
  assert(a == b && b == c && c == d);

  interner_free(&baba);
  vmarena_free(&vmarena);
}
