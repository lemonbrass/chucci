#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <thirdparty/kvec.h>
#include <memscope.h>


MemScope* new_scope() {
  MemScope* scope = malloc(sizeof(MemScope));
  *scope = (MemScope){0};
  return scope;
}

inline MemEntry new_mem_entry(void* ptr, MemDestructorFn fn) {
  return (MemEntry){ .destructor=fn, .ptr=ptr };
}

void track_mem(MemScope* scope, void* ptr, MemDestructorFn fn) {
  kv_push(MemEntry, *scope, new_mem_entry(ptr, fn));
}

void untrack_mem(MemScope* scope, void* ptr) {
  for (size_t i=0; i < kv_size(*scope); i++) {
    MemEntry* entry = &kv_A(*scope, i);
    if (entry->ptr == ptr) {
      *entry = (MemEntry){0};
      return;
    }
  }
  assert(false && "Ptr doesnt exist, cant untrack");
}

void free_scope(MemScope** scope) {
  while (kv_size(**scope) > 0) {
    MemEntry entry = kv_pop(**scope);
    entry.destructor(entry.ptr);
  }
  kv_destroy(**scope);
  free(*scope);
  *scope = NULL;
}
