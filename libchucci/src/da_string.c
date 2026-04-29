#include <assert.h>
#include <da_string.h>
#include <stdio.h>
#include <string.h>

string_view new_sv(const char* cstr, size_t len) {
  return (string_view){.cstr = cstr, .len = len};
}

string new_str(const char* owned, size_t len) {
  return (string){.cstr = owned, .len = len};
}

da_string new_ds() {
  da_string ds = {0};
  return ds;
}

string_view sv_from_cstr(const char* cstr) {
  return (string_view) {.cstr = cstr, .len = strlen(cstr)};
}

void free_ds(da_string* ds) {
  free(ds->cstr);
  *ds = (da_string){0};
}

void free_str(string* str) {
  free((void*)str->cstr);
  *str = (string){0};
}


void reset_ds(da_string* ds) {
  memset(ds->cstr, 0, ds->len);
  ds->len = 0;
}

string_view sv_slice_till_delim(string_view sv, char delim) {
  for (size_t i=0; i<sv.len; i++) {
    if (sv.cstr[i] == delim) {
      return sv_slice(sv, 0, i);
    }
  }
  assert(false && "Delim not found");
}

string_view sv_slice(string_view sv, size_t start, size_t len) {
  assert(sv.len >= start + len);
  return new_sv(sv.cstr + start, len);
}

void grow_ds(da_string* ds, size_t cap) {
  assert(cap!=0 && cap != ds->cap);
  char* new_str = realloc(ds->cstr, cap);
  assert(new_str != NULL);
  ds->cap = cap;
  ds->cstr = new_str;
}

void push_ds(da_string* ds, string_view sv) {
  size_t cap = (ds->cap == 0) ? DS_DEFAULT_CAPACITY : ds->cap;
  while (cap <= ds->len + sv.len + 1) cap *= 2;
  if (cap != ds->cap) grow_ds(ds, cap);

  memcpy(ds->cstr + ds->len, sv.cstr, sv.len);
  ds->len += sv.len;
}

void push_char_ds(da_string* ds, char ch) {
  size_t cap = (ds->cap == 0) ? DS_DEFAULT_CAPACITY : ds->cap;
  while (ds->cap <= ds->len + 1) grow_ds(ds, cap*2);
  ds->cstr[ds->len] = ch;
  ds->len++;
}

// allocates a new string
string build_ds(da_string* ds) {
  string str;
  char* cstr = malloc(ds->len+1);
  assert(cstr != NULL);
  memcpy(cstr, ds->cstr, ds->len);
  str.len = ds->len;
  str.cstr = cstr;
  return str;
}


string_view ds_to_sv(da_string* ds) {
  return new_sv(ds->cstr, ds->len);
}
string_view str_to_sv(string str) {
  return new_sv(str.cstr, str.len);
}
//copies cstr because string type needs owned memory by design
string ds_to_str_copy(da_string* ds) {
  char* cstr = malloc(ds->len);
  assert(cstr != NULL);
  memcpy(cstr, ds->cstr, ds->len);
  return new_str(cstr, ds->len);
}
string sv_to_str_copy(string_view sv) {
  char* cstr = malloc(sv.len);
  assert(cstr != NULL);
  memcpy(cstr, sv.cstr, sv.len);
  return new_str(cstr, sv.len);
}
da_string str_to_ds_copy(string str) {
  char* cstr = malloc(str.len);
  assert(cstr != NULL);
  memcpy(cstr, str.cstr, str.len);
  return (da_string){ .cstr=cstr, .cap=str.len, .len=str.len};
}
da_string sv_to_ds_copy(string_view sv) {
  char* cstr = malloc(sv.len);
  assert(cstr != NULL);
  memcpy(cstr, sv.cstr, sv.len);
  return (da_string){ .cstr=cstr, .cap=sv.len, .len=sv.len};
}

