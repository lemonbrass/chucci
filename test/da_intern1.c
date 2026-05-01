#include <da_string.h>
#include <da_intern.h>
#include <setjmp.h>


void da_intern1(jmp_buf errbuf) {
  InternTable* table = new_interntable();
  string_view str1 = sv_from_cstr("hi");
  string_view str2 = sv_from_cstr("hi");
  string_view str3 = sv_from_cstr("hello");
  interned_str istr1 = intern(table, str1);
  interned_str istr2 = intern(table, str2);
  interned_str istr3 = intern(table, str3);
  if(!interned_eq(intern(table, str1), intern(table, str2))
     || !interned_eq(intern(table, str2), istr2)
     || !interned_eq(intern(table, str3), istr3)) {
    free_interntable(&table);
    longjmp(errbuf, 1);
  }
  free_interntable(&table);
}
