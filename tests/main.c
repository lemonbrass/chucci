#include "utils/chucci_alloc.h"
// #include "utils/chunked_arena.h"
#include "utils/vec.h"
#include "utils/vmem_arena.h"
#include <stdio.h>

VEC_DEF(int, IntVec, ints);
VEC_IMPL(int, IntVec, ints, VMEM_ARENA_ALLOC_INT);

#define VEC_LEN 1024

int main() {
  // ChunkedArena charena = chnk_arena_new();
  VMEMArena vmarena = vmarena_new();

  IntVec vec = ints_new();
  ints_resize(&vec, VEC_LEN, &vmarena);

  for (size_t i = 0; i < VEC_LEN; i++)
    ints_push(&vec, i, &vmarena);

  vec_foreach(int, &vec, i, num, { printf("%zu: %d, ", i, num); });
  printf("\n");

  vmarena_free(&vmarena);
  // chnk_arena_free(&charena);
}
