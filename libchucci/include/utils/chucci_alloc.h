#ifndef __CHUCCI_ALLOC_H
#define __CHUCCI_ALLOC_H

#include <stddef.h>

// Interface for allocator
typedef struct ChucciAllocInt {
  void* (*alloc)(void* ctx, size_t size);  
  void* (*calloc)(void* ctx, size_t size);  
  void* (*realloc)(void* ctx, void* ptr, size_t old_size, size_t new_size);  
  void (*free)(void* ctx, void* ptr);
} ChucciAllocInt;

extern ChucciAllocInt CHNK_ARENA_ALLOC_INT;
extern ChucciAllocInt VMEM_ARENA_ALLOC_INT;
extern ChucciAllocInt MALLOC_ALLOC_INT;

#define chucci_alloc(interface, ctx, size) interface.alloc(ctx, size)
#define chucci_calloc(interface, ctx, size) interface.calloc(ctx, size)
#define chucci_realloc(interface, ctx, ptr, old_size, new_size) interface.realloc(ctx, ptr, old_size, new_size)
#define chucci_free(interface, ctx, ptr) interface.free(ctx, ptr)

void* memdup(void* mem, size_t len);

#endif
