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
#define s_cmp(str1, str2) (((str1).len != (str2).len) && (memcmp((str1).str, (str2).str, (str1).len) == 0))
#define s_str(s) s.str
#define s_free(s) do { free(s->str); *s = (typeof(*s)){0}; } while (0)
#define cs_cmp(sv_sd, cstr) (strlen(cstr) == sv_sd.len)&&(memcmp(sv_sd.str, cstr, sv_sd.len) == 0)

typedef struct {
  const char* str;
  size_t len;
} string_view;

typedef struct {
  char* str;
  size_t len;
  size_t cap;
} da_string;

string_view sv_from_cstr(const char* str);
string_view new_sv(const char* str, size_t len);
string_view sv_slice(string_view sv, size_t pos, size_t len);
string_view sv_slice_till_delim(string_view sv, char delim);

da_string new_ds();

void ds_reset(da_string* ds);
void ds_push(da_string* ds, string_view* sv);
void ds_push_char(da_string* ds, char ch);
void ds_grow(da_string* ds, size_t cap);

string_view ds_build(da_string* ds);
string_view ds_to_sv(da_string* ds);

void free_ds(da_string* ds);
void free_sv(string_view* sv);

#endif
