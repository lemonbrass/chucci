#ifndef __STRING_H
#define __STRING_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

typedef struct StringView {
  char* cstr;
  size_t len;
} StringView;

typedef struct String {
  char* cstr;
  size_t len;
} String;


#define cstr_to_anystr(_cstr, STR_T) (STR_T){ .cstr = _cstr, .len = strlen(_cstr) }
#define anystr_to_sv(str) (StringView){ .cstr = (str).cstr, .len = (str).len }
#define anystr_to_str(str) (String){ .cstr = (str).cstr, .len = (str).len }
#define anystr_to_sb(str) (StringBuilder){ .cstr = (str).cstr, .len = (str).len }

#define print_str(str) printf("%.*s", (str).len, (str).cstr)
#define println_str(str) printf("%.*s\n", (str).len, (str).cstr)
#define len(str) (str).len

#define str_eq(str1, str2) ((str1).len == (str2).len && memcmp((str1).cstr, (str2).cstr, (str1).len) == 0)
#define str_startswith(str, ch) (assert((str).len > 0), (str).cstr[0] == ch)
#define str_trim(str, sv_result) do {\
  (sv_result) = anystr_to_sv((str));\
  while ((sv_result).len > 0 && isspace( (unsigned char)(sv_result).cstr[0]) ) {\
    (sv_result).len--;\
    (sv_result).cstr++;\
  }\
  while ((sv_result).len > 0 && isspace( (unsigned char)(sv_result)[(sv_result).len - 1]) ) (sv_result).len--;\
} while (0)  
#define str_consume_while(str, sv_result, func) do {\
  (sv_result) = anystr_to_sv((str));\
  while ((sv_result).len > 0 && func( (sv_result).cstr[0] )) {\
    (sv_result).len--;\
    (sv_result)->cstr++;\
  }\
} while (0)

#define FNV_OFFSET_BASIS 14695981039346656037ULL
#define FNV_PRIME 1099511628211ULL

static inline uint64_t hash_string(const StringView sv) {
  uint64_t hash = FNV_OFFSET_BASIS;
  for (size_t i = 0; i < sv.len; i++) {
    hash ^= (uint64_t)(unsigned char)sv.cstr[i];
    hash *= FNV_PRIME;
  }
  return hash;
}

#endif
