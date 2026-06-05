#ifndef __SMALLVEC_H
#define __SMALLVEC_H


#include <stddef.h>
#include <assert.h>

#define SMALLVEC_RESIZE_RATIO 2
#define SMALLVEC_DEFAULT_CAP 8

#ifdef SMALLVEC_DEBUG
#define SMALLVEC_DEBUG_FIELD size_t resize_count;
#define SMALLVEC_DEBUG_INCREASE_RESIZE_COUNT(vec) vec->resize_count++
#define SMALLVEC_DEBUG_LOG_RESIZES(name, vec) \
  printf("Vector %s resized, { .data=%p, .len=%zu, .cap=%zu, .resize_count=%zu}",\
         #name, vec->data, vec->len, vec->cap, vec->resize_count)
#else
#define SMALLVEC_DEBUG_FIELD
#define SMALLVEC_DEBUG_INCREASE_RESIZE_COUNT(vec)
#define SMALLVEC_DEBUG_LOG_RESIZES(name, vec)
#endif

#define SMALLVEC_DEF(T, name, prefix)\
typedef struct name {\
  union {\
    T* data;\
    T s_data[SMALLVEC_DEFAULT_CAP];\
  };\
  size_t len;\
  struct {\
    uint64_t cap : 63;\
    uint64_t is_static : 1;\
  };\
  SMALLVEC_DEBUG_FIELD\
} name;\
name prefix##_new();\
void prefix##_resize(name* vec, size_t new_cap, void* ctx);\
void prefix##_free(name* vec, void* ctx);\
T prefix##_pop(name* vec);\
T prefix##_top(name* vec);\
T prefix##_access(name* vec, size_t i);\
T* prefix##_top_ptr(name* vec);\
T* prefix##_access_ptr(name* vec, size_t i);\
size_t prefix##_len(name* vec);\
size_t prefix##_cap(name* vec);\
void prefix##_push(name* vec, T val, void* ctx);\

#define SMALLVEC_DEF_WITH_FIELDS(T, name, prefix, fields)\
typedef struct name {\
  union {\
    T* data;\
    T s_data[SMALLVEC_DEFAULT_CAP];\
  };\
  size_t len;\
  struct {\
    uint64_t cap : 63;\
    uint64_t is_static : 1;\
  };\
  SMALLVEC_DEBUG_FIELD\
} name;\
name prefix##_new();\
void prefix##_resize(name* vec, size_t new_cap, void* ctx);\
void prefix##_free(name* vec, void* ctx);\
T prefix##_pop(name* vec);\
T prefix##_top(name* vec);\
T prefix##_access(name* vec, size_t i);\
T* prefix##_top_ptr(name* vec);\
T* prefix##_access_ptr(name* vec, size_t i);\
size_t prefix##_len(name* vec);\
size_t prefix##_cap(name* vec);\
void prefix##_push(name* vec, T val, void* ctx);\
void prefix##_reset(name* vec);\

#define SMALLVEC_IMPL(T, name, prefix, alloc_int)\
name prefix##_new() {\
  name vec = (name){0};\
  vec.is_static = 1;\
  vec.cap = SMALLVEC_DEFAULT_CAP;\
  return vec;\
}\
void prefix##_resize(name* vec, size_t new_cap, void* ctx) {\
  SMALLVEC_DEBUG_INCREASE_RESIZE_COUNT(vec);\
  vec->data = chucci_realloc(alloc_int, ctx, vec->data, vec->cap*sizeof(T), new_cap*sizeof(T));\
  vec->cap = new_cap;\
  vec->is_static = 0;\
  SMALLVEC_DEBUG_LOG_RESIZES(name, vec);\
}\
void prefix##_free(name* vec, void* ctx) {\
  if (vec->data && !vec->is_static) chucci_free(alloc_int, ctx, vec->data);\
  *vec = (name){0};\
}\
T prefix##_pop(name* vec) {\
  assert(vec->len > 0);\
  if (vec->is_static) return vec->s_data[--vec->len];\
  return vec->data[--vec->len];\
}\
T prefix##_top(name* vec) {\
  assert(vec->len > 0);\
  if (vec->is_static) return vec->s_data[vec->len-1];\
  return vec->data[vec->len-1];\
}\
void prefix##_push(name* vec, T val, void* ctx) {\
  if (vec->len+1 > vec->cap)\
    vec->cap ? prefix##_resize(vec, vec->cap * SMALLVEC_RESIZE_RATIO, ctx) : prefix##_resize(vec, SMALLVEC_DEFAULT_CAP, ctx);\
  if (vec->is_static) vec->s_data[vec->len++] = val;\
  else vec->data[vec->len++] = val;\
}\
size_t prefix##_cap(name* vec) {\
  return vec->cap;\
}\
size_t prefix##_len(name* vec) {\
  return vec->len;\
}\
T prefix##_access(name* vec, size_t i) {\
  if (vec->is_static) vec->s_data[i];\
  return vec->data[i];\
}\
T* prefix##_top_ptr(name* vec) {\
  assert(vec->len > 0);\
  if (vec->is_static) return &vec->s_data[vec->len-1];\
  return &vec->data[vec->len-1];\
}\
void prefix##_reset(name* vec) {\
  vec->len = 0;\
}\
T* prefix##_access_ptr(name* vec, size_t i) {\
  if (vec->is_static) &vec->s_data[i];\
  return &vec->data[i];\
}

#define smallvec_foreach(T, vec, idx, element, body) for (size_t idx=0; i<(vec)->len; idx++) { T element = (vec)->data[idx]; body;}

#endif
