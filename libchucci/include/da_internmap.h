#ifndef DA_INTERN_MAP
#define DA_INTERN_MAP

#include <thirdparty/kvec.h>
#include <da_bitset.h>

/*
  This is not a hashmap, but acts like one..
  This works only because interned_str have a corresponding dense id
  However, this is very fast & memory efficient
*/

#define internedmap_t(T) struct{ kvec_t(T) data; bitset_t bitset; }
#define imap_init(imap) (kv_init(imap.data), imap.bitset = new_bs(64))
#define imap_has(imap, str) (get_bit(&(imap.bitset), str.id) == 1)
#define imap_get(imap, istr) (imap_has(imap, istr) ? &kv_A((imap).data, (istr).id) : NULL)
#define imap_set(val, imap, istr) (kv_a(typeof(val), (imap).data, (istr).id), kv_A(imap.data, istr.id) = val, set_bit(&(imap).bitset, (istr).id))
#define imap_destroy(imap) (kv_destroy((imap).data), free_bs(&(imap).bitset))

#endif
