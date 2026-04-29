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
#define s_print(s) printf("%.*s", (int)s.len, s.cstr)
#define s_println(s) printf("%.*s\n", (int)s.len, s.cstr)
#define s_cmp(str1, str2) (((str1).len != (str2).len) && (memcmp((str1).cstr, (str2).cstr, (str1).len) == 0))
#define cs_cmp(str, cstr) (strlen(cstr) == str.len)&&(memcmp(str.cstr, cstr, str.len) == 0)

// Immutable and DOESNT OWN the memory
typedef struct {
  const char* cstr;
  size_t len;
} string_view;

// Mutable and owns the memory
typedef struct {
  char* cstr;
  size_t len;
  size_t cap;
} da_string;

// Immutable but owns the memory
typedef struct {
  const char* cstr;
  size_t len;
} string;

/*
*   It is recommended to pass string_view and string by value and da_string by ptr
*   string_view and string are only different in design philosophies (owned vs borrowed)
*/

string_view sv_from_cstr(const char* str);
string_view sv_slice(string_view sv, size_t pos, size_t len);
string_view sv_slice_till_delim(string_view sv, char delim);

string new_str(const char* owned, size_t len);
string_view new_sv(const char* str, size_t len);
da_string new_ds();

void reset_ds(da_string* ds);
void push_ds(da_string* ds, string_view sv);
void push_char_ds(da_string* ds, char ch);
void grow_ds(da_string* ds, size_t cap);

string build_ds(da_string* ds);

void free_ds(da_string* ds);
// This is an exception to that recommendation because this function sets str to null after freeing
void free_str(string* str);


// Conversions
// string_view doesnt own the memory, and hence, anything can be converted to it without copying
string_view ds_to_sv(da_string* ds);
string_view str_to_sv(string str);
string ds_to_str_copy(da_string* ds);
string sv_to_str_copy(string_view sv);
da_string str_to_ds_copy(string str);
da_string sv_to_ds_copy(string_view sv);

#endif
