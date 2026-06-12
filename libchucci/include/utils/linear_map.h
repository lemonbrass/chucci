#ifndef __LINEAR_MAP_H
#define __LINEAR_MAP_H

#include <stddef.h>
#include "chucci_alloc.h"

#define DEFAULT_LINEAR_MAP_CAP 8


#define LINEAR_MAP_DEF(K, V, name, prefix, alloc_int)\
typedef struct {\
  K k;\
  V v;\
} __##prefix##_entry;\
SMALLVEC_DEF(__##prefix##_entry, name, __##prefix, alloc_int)\
name prefix##_new();\
void prefix##_free(name *map, void *ctx);\
V *prefix##_get(name *map, K k);\
void prefix##_set(name *map, K k, V v, void *ctx);

#define LINEAR_MAP_IMPL(K, V, name, prefix, alloc_int)\
SMALLVEC_IMPL(__##prefix##_entry, name, __##prefix, alloc_int)\
name prefix##_new() {\
  name map = {0};\
  return map;\
}\
void prefix##_free(name *map, void *ctx) {\
  __##prefix##_free(map, ctx);\
}\
V *prefix##_get(name *map, K k) {\
  size_t i = 0;\
  __##prefix##_entry *entry;\
  while (i < map->len && (entry = __##prefix##_access_ptr(map, i++))->k != k);\
  return i < map->len ? &entry->v : NULL;\
}\
void prefix##_set(name *map, K k, V v, void *ctx){\
  __##prefix##_push(map, (__##prefix##_entry){.k = k, .v = v}, ctx);\
}



#endif
