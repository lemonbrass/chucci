#ifndef DA_INTERN_MAP
#define DA_INTERN_MAP

#include <thirdparty/kvec.h>

/*
  This is not a hashmap, but acts like one..
  This works only because interned_str have a corresponding dense id
  However, this is very fast & memory efficient
*/

#define internedmap_t(T) struct{ kvec_t(T) data; }
#define imap_get(imap, istr) kv_A((imap).data, (istr).id)
#define imap_set(val, imap, istr) (kv_a(typeof(val), (imap).data, (istr).id), kv_A(imap.data, istr.id) = val)
#define imap_destroy(imap) kv_destroy((imap).data)

#endif
