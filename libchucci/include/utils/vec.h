#ifndef __VEC_H
#define __VEC_H

#include <stddef.h>
#include <assert.h>

#define VEC_RESIZE_RATIO 2
#define VEC_DEFAULT_CAP 8

#ifdef VEC_DEBUG
#include <stdio.h>
#define VEC_DEBUG_FIELD size_t resize_count;
#define VEC_DEBUG_INCREASE_RESIZE_COUNT(vec) vec->resize_count++
#define VEC_DEBUG_LOG_RESIZES(name, vec) \
  printf("Vector %s resized, { .data=%p, .len=%zu, .cap=%zu, .resize_count=%zu}",\
         #name, vec->data, vec->len, vec->cap, vec->resize_count)
#else
#define VEC_DEBUG_FIELD
#define VEC_DEBUG_INCREASE_RESIZE_COUNT(vec)
#define VEC_DEBUG_LOG_RESIZES(name, vec)
#endif

#define __VEC_DEF_FN(T, name, prefix)\
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
void prefix##_reset(name* vec);

#define VEC_DEF(T, name, prefix)\
typedef struct name {\
  T* data;\
  size_t len, cap;\
  VEC_DEBUG_FIELD\
} name;\
__VEC_DEF_FN(T, name, prefix)

#define VEC_DEF_WITH_FIELDS(T, name, prefix, fields)\
typedef struct name {\
  T* data;\
  size_t len, cap;\
  VEC_DEBUG_FIELD\
  fields\
} name;\
__VEC_DEF_FN(T, name, prefix)

#define VEC_IMPL(T, name, prefix, alloc_int)\
name prefix##_new() {\
  return (name){0};\
}\
void prefix##_resize(name* vec, size_t new_cap, void* ctx) {\
  VEC_DEBUG_INCREASE_RESIZE_COUNT(vec);\
  vec->data = chucci_realloc(alloc_int, ctx, vec->data, vec->cap*sizeof(T), new_cap*sizeof(T));\
  vec->cap = new_cap;\
  VEC_DEBUG_LOG_RESIZES(name, vec);\
}\
void prefix##_free(name* vec, void* ctx) {\
  if (vec->data) chucci_free(alloc_int, ctx, vec->data);\
  *vec = (name){0};\
}\
T prefix##_pop(name* vec) {\
  assert(vec->len > 0);\
  return vec->data[--vec->len];\
}\
T prefix##_top(name* vec) {\
  assert(vec->len > 0);\
  return vec->data[vec->len-1];\
}\
void prefix##_push(name* vec, T val, void* ctx) {\
  if (vec->len+1 > vec->cap)\
    vec->cap ? prefix##_resize(vec, vec->cap * VEC_RESIZE_RATIO, ctx) : prefix##_resize(vec, VEC_DEFAULT_CAP, ctx);\
  vec->data[vec->len++] = val;\
}\
size_t prefix##_cap(name* vec) {\
  return vec->cap;\
}\
size_t prefix##_len(name* vec) {\
  return vec->len;\
}\
T prefix##_access(name* vec, size_t i) {\
  return vec->data[i];\
}\
T* prefix##_top_ptr(name* vec) {\
  assert(vec->len > 0);\
  return &vec->data[vec->len-1];\
}\
void prefix##_reset(name* vec) {\
  vec->len = 0;\
}\
T* prefix##_access_ptr(name* vec, size_t i) {\
  return &vec->data[i];\
}

#define vec_foreach(T, vec, idx, element, body) for (size_t idx=0; idx<(vec)->len; idx++) { T element = (vec)->data[idx]; body;}

#endif
