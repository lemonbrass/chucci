#ifndef MEMSCOPE_H
#define MEMSCOPE_H

#include <stddef.h>
#include <thirdparty/kvec.h>

typedef void (*MemDestructorFn)(void*);

typedef struct {
  void* ptr;
  MemDestructorFn destructor;
} MemEntry;

typedef kvec_t(MemEntry) MemScope;

MemScope* new_scope();
void track_mem(MemScope* scope, void* ptr, MemDestructorFn fn);
void untrack_mem(MemScope* scope, void* ptr);
void free_scope(MemScope** scope);

#endif
