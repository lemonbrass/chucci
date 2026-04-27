#ifndef DA_STRING_H
#define DA_STRING_H

#include <stdint.h>
#ifndef DS_DEFAULT_CAPACITY
#define DS_DEFAULT_CAPACITY 8
#endif

#include <da_arena.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#define s_len(str) str.len
#define s_print(s) printf("%.*s", (int)s.len, s.str)
#define s_cmp(str1, str2) (((str1).len != (str2).len) ? false : (strncmp((str1).str, (str2).str, (str1).len) == 0))
#define s_str(s) s.str

#define cs_cmp(sv_sd, cstr) (strlen(cstr) == sv_sd.len)&&(strncmp(sv_sd.str, cstr, strlen(cstr)) == 0)

typedef struct {
  const char* str;
  size_t len;
} string_view;

typedef struct {
  char* str;
  size_t len;
  size_t cap;
  arena* ar;
} da_string;

string_view sv_new(const char* str, size_t len);
da_string ds_new(arena* ar);
void ds_push(da_string* ds, string_view* sv);
void ds_push_char(da_string* ds, char ch);
void ds_grow(da_string* ds, size_t cap);
string_view ds_build(da_string* ds);
string_view dstosv(da_string* ds);

#endif
