#ifndef __VMEM_ARENA_H
#define __VMEM_ARENA_H

#include <stddef.h>
#include <stdint.h>

#define VMEM_ARENA_MAX_CAP 1024 * 1024

typedef struct VMEMArena {
  uint8_t *data;
  size_t pos, cap;
} VMEMArena;

typedef struct VMEMArenaMark {
  size_t pos;
} VMEMArenaMark;

VMEMArena *vmarena_new(size_t cap);
void *vmarena_alloc(VMEMArena *arena, size_t size);
void *vmarena_calloc(VMEMArena *arena, size_t size);
void *vmarena_realloc(VMEMArena *arena, void *ptr, size_t old_size, size_t new_size);
void vmarena_reset(VMEMArena *arena);
void vmarena_mark_reset(VMEMArena *arena, VMEMArenaMark mark);
VMEMArenaMark vmarena_mark(VMEMArena *arena);
void vmarena_free(VMEMArena *arena);

#endif
