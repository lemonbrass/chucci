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
  ds.cap = DS_SSO_CAP;
  return ds;
}

bool is_ds_sso(da_string* ds) {
  return DS_SSO_CAP == ds->cap;
}

char* get_cstr_from_ds(da_string* ds) {
  if (is_ds_sso(ds)) return ds->_sso;
  else return ds->_cstr;
}

string str_from_cstr_copy(const char* cstr) {
  return new_str(strdup(cstr), strlen(cstr));
}

string_view sv_from_cstr(const char* cstr) {
  return (string_view) {.cstr = cstr, .len = strlen(cstr)};
}

void free_ds(da_string* ds) {
  if (!is_ds_sso(ds)) free(ds->_cstr);
  *ds = new_ds();
}

void free_str(string* str) {
  free((void*)str->cstr);
  *str = (string){0};
}


void reset_ds(da_string* ds) {
  if (is_ds_sso(ds)) memset(ds->_sso, 0, ds->len);
  else memset(ds->_cstr, 0, ds->len);
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
  if (ds->cap == cap) return;
  assert(cap!=0);
  // we were using _cstr and have to resize
  if (cap > DS_SSO_CAP && !is_ds_sso(ds)) {
    char* new_str = realloc(ds->_cstr, cap);
    assert(new_str != NULL);
    ds->cap = cap;
    ds->_cstr = new_str;
  }
  // we were using sso, but now cant because sso is too small
  else if (cap > DS_SSO_CAP && is_ds_sso(ds)) {
    char* new_str = malloc(cap);
    assert(new_str != NULL);
    memcpy(new_str, ds->_sso, ds->len);
    ds->cap = cap;
    ds->_cstr = new_str;
  }
  // we can still use sso
  else {
    ds->cap = DS_SSO_CAP;
    printf("shouldnt be so\n");
  }
}

void push_ds(da_string* ds, string_view sv) {
  size_t cap = (ds->cap == 0) ? DS_DEFAULT_CAPACITY : ds->cap;
  while (cap <= ds->len + sv.len) cap *= 2;
    if (cap != ds->cap) grow_ds(ds, cap);
  // if ds is already allocared on heap
  memcpy(get_cstr_from_ds(ds) + ds->len, sv.cstr, sv.len);

  ds->len += sv.len;
}

void push_char_ds(da_string* ds, char ch) {
  size_t cap = (ds->cap == 0) ? DS_DEFAULT_CAPACITY : ds->cap;
  while (cap <= ds->len + 1) cap *= 2;
  grow_ds(ds, cap);
  get_cstr_from_ds(ds)[ds->len] = ch;
  ds->len++;
}

// allocates a new string
string build_ds(da_string* ds) {
  return ds_to_str_copy(ds);
}


string_view ds_to_sv(da_string* ds) {
  return new_sv(get_cstr_from_ds(ds), ds->len);
}
string_view str_to_sv(string str) {
  return new_sv(str.cstr, str.len);
}
//copies cstr because string type needs owned memory by design
string ds_to_str_copy(da_string* ds) {
  char* cstr = malloc(ds->len);
  assert(cstr != NULL);
  memcpy(cstr, get_cstr_from_ds(ds), ds->len);
  return new_str(cstr, ds->len);
}
string sv_to_str_copy(string_view sv) {
  char* cstr = malloc(sv.len);
  assert(cstr != NULL);
  memcpy(cstr, sv.cstr, sv.len);
  return new_str(cstr, sv.len);
}
da_string str_to_ds_copy(string str) {
  if (str.len > DS_SSO_CAP) {
    da_string ds;
    ds.cap = str.len;
    ds.len = str.len;
    ds._cstr = malloc(str.len);
    assert(ds._cstr != NULL);
    memcpy(ds._cstr, str.cstr, str.len);
    return ds;
  } else {
    da_string ds;
    ds.cap = DS_SSO_CAP;
    ds.len = str.len;
    memcpy(ds._sso, str.cstr, str.len);
    return ds;
  }
}
da_string sv_to_ds_copy(string_view sv) {
  da_string ds;
  if (sv.len > DS_SSO_CAP) {
    ds.cap = sv.len;
    ds.len = sv.len;
    ds._cstr = malloc(sv.len);
    assert(ds._cstr != NULL);
    memcpy(ds._cstr, sv.cstr, sv.len);
  }
  else {
    ds.cap = DS_SSO_CAP;
    ds.len = sv.len;
    memcpy(ds._sso, sv.cstr, sv.len);
  }
  return ds;
}

