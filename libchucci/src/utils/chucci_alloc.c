#include "utils/chucci_alloc.h"
#include "utils/chunked_arena.h"
#include "utils/vmem_arena.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static void *_malloc(void *ctx, size_t size) { return malloc(size); }
static void *_calloc(void *ctx, size_t size) { return calloc(size, 1); }
static void _free(void *ctx, void *ptr) { free(ptr); }
static void *_realloc(void *ctx, void *ptr, size_t old_size, size_t new_size) {
  return realloc(ptr, new_size);
}

static void no_op(void *_a, void *_b) {
  (void)_a;
  (void)_b;
}

ChucciAllocInt MALLOC_ALLOC_INT = (ChucciAllocInt){
    .alloc = _malloc, .calloc = _calloc, .realloc = _realloc, .free = _free};

ChucciAllocInt CHNK_ARENA_ALLOC_INT =
    (ChucciAllocInt){.alloc = (void *)chnk_arena_alloc,
                     .free = no_op,
                     .realloc = (void *)chnk_arena_realloc,
                     .calloc = (void *)chnk_arena_calloc};

ChucciAllocInt VMEM_ARENA_ALLOC_INT =
    (ChucciAllocInt){.alloc = (void *)vmarena_alloc,
                     .free = no_op,
                     .realloc = (void *)vmarena_realloc,
                     .calloc = (void *)vmarena_calloc};
