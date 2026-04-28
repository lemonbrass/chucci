#include <assert.h>
#include <da_string.h>
#include <stdio.h>
#include <string.h>

string_view new_sv(const char* str, size_t len) {
  return (string_view){.str = str, .len = len};
}

string_view sv_from_cstr(const char* str) {
  return (string_view) {.str = str, .len = strlen(str)};
}

string_view sv_slice_till_delim(string_view sv, char delim) {
  for (size_t i=0; i<sv.len; i++) {
    if (sv.str[i] == delim) {
      return sv_slice(sv, 0, i);
    }
  }
  assert(false && "Delim not found");
}

string_view sv_slice(string_view sv, size_t start, size_t len) {
  assert(sv.len >= start + len);
  return new_sv(sv.str + start, len);
}

void ds_grow(da_string* ds, size_t cap) {
  char* new_str = realloc(ds->str, cap);
  assert(new_str != NULL);
  ds->cap = cap;
  ds->str = new_str;
}

da_string new_ds() {
  da_string ds = {0};
  return ds;
}

void ds_push(da_string* ds, string_view* sv) {
  size_t cap = (ds->cap == 0) ? DS_DEFAULT_CAPACITY : ds->cap;
  while (cap <= ds->len + sv->len + 1) cap *= 2;
  if (cap != ds->cap) ds_grow(ds, cap);

  strncpy(ds->str + ds->len, sv->str, sv->len);
  ds->len += sv->len;
}

void ds_push_char(da_string* ds, char ch) {
  while (ds->cap <= ds->len + 1) ds_grow(ds, ds->cap*2);
  ds->str[ds->len] = ch;
  ds->len++;
}

// doesnt alloc, borrowed string view
string_view ds_to_sv(da_string* ds) {
  return new_sv(ds->str, ds->len);
}

// allocates a new string
string_view ds_build(da_string* ds) {
  string_view sv;
  char* str = malloc(ds->len+1);
  assert(str != NULL);
  strncpy(str, ds->str, ds->len);
  sv.len = ds->len;
  sv.str = str;
  return sv;
}

void free_ds(da_string* ds) {
  free(ds->str);
  *ds = (da_string){0};
}

void free_sv(string_view* sv) {
  free((void*)sv->str);
  *sv = (string_view){0};
}

