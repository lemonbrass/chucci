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
K *prefix##_get_key(name *map, V v);\
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
  for (size_t i = 0; i < map->len; i++) {\
    __##prefix##_entry *entry = __##prefix##_access_ptr(map, i);\
    if (entry->k == k) {\
      return &entry->v;\
    }\
  }\
  return NULL;\
}\
void prefix##_set(name *map, K k, V v, void *ctx){\
  for (size_t i = 0; i < map->len; i++) {\
    __##prefix##_entry *entry = __##prefix##_access_ptr(map, i);\
    if (memcmp(&entry->k, &k, sizeof(k)) == 0) {\
      entry->v = v;\
      return;\
    }\
  }\
  __##prefix##_push(map, (__##prefix##_entry){.k = k, .v = v}, ctx);\
}\
K *prefix##_get_key(name *map, V v){\
  for (size_t i = 0; i < map->len; i++) {\
    __##prefix##_entry *entry = __##prefix##_access_ptr(map, i);\
    if (memcmp(&entry->v, &v, sizeof(v)) == 0) {\
      return &entry->k;\
    }\
  }\
  return NULL;\
}




#endif
