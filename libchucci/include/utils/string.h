#ifndef __STRING_H
#define __STRING_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct StringView {
  char* cstr;
  size_t len;
} StringView;

// String is always allocated from malloc
// because arena allocated strings are always represented
// as StringView
typedef struct String {
  char* cstr;
  size_t len;
} String;


#define cstr_to_sv(_cstr) (StringView){ .cstr = (_cstr), .len = strlen(_cstr) }
#define const_cstr_to_sv(_cstr) (StringView){ .cstr = (_cstr), .len = sizeof(_cstr) - 1 }
#define cstr_to_str(_cstr) (String){ .cstr = strdup(_cstr), .len = strlen((_cstr)) }
#define const_cstr_to_str(_cstr) (String){ .cstr = strdup(_cstr), .len = sizeof(_cstr) - 1 }
#define anystr_to_sv(str) (StringView){ .cstr = (str).cstr, .len = (str).len }
#define anystr_to_str(str) (String){ .cstr = memdup((str).cstr, (str).len), .len = (str).len }
#define str_new(_cstr, _len) (String){ .cstr = memdup((_cstr), (_len)), .len = (_len) }
#define str_free(str) free((str).cstr)
#define sv_new(_cstr, _len) (StringView){ .cstr = (_cstr), .len = (_len) }

#define anystr_print(str) printf("%.*s", (int)(str).len, (str).cstr)
#define anystr_println(str) printf("%.*s\n", (int)(str).len, (str).cstr)
#define len(str) (str).len

#define anystr_slice(str, start, end) (StringView){.cstr = (str).cstr+(start), .len = ((end) - (start))}
#define anystr_eq(str1, str2) ((str1).len == (str2).len && memcmp((str1).cstr, (str2).cstr, (str1).len) == 0)
#define anystr_startswith(str, ch) (assert((str).len > 0), (str).cstr[0] == ch)
#define anystr_trim(str, sv_result) do {\
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
