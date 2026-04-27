#include <assert.h>
#include <da_string.h>
#include <string.h>

string_view sv_new(const char* str, size_t len) {
  return (string_view){.str = str, .len = len};
}

void ds_grow(da_string* ds, size_t cap) {
  char* new_str = arena_alloc(ds->ar, cap);
  ds->cap = cap;
  strncpy(new_str, ds->str, ds->len);
  ds->str = new_str;
}

da_string ds_new(arena* ar) {
  da_string ds = {0};
  ds.ar = ar;
  return ds;
}

void ds_push(da_string* ds, string_view* sv) {
  size_t cap = (ds->cap == 0) ? DS_DEFAULT_CAPACITY : ds->cap;
  if (cap <= ds->len + sv->len) cap *= 2;
  if (cap != ds->cap) ds_grow(ds, cap);

  strncpy(ds->str + ds->len, sv->str, sv->len);
  ds->len += sv->len;
}

void ds_push_char(da_string* ds, char ch) {
  if (ds->cap <= ds->len + 1) ds_grow(ds, ds->cap*2);
  ds->str[ds->len] = ch;
  ds->len++;
}

// doesnt alloc, borrowed string view
string_view dstosv(da_string* ds) {
  return sv_new(ds->str, ds->len);
}

// allocates a new string
string_view ds_build(da_string* ds) {
  string_view sv;
  char* str = arena_alloc(ds->ar, ds->len);
  strncpy(str, ds->str, ds->len);
  sv.len = ds->len;
  sv.str = str;
  return sv;
}



