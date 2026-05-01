#include <da_string.h>
#include <cursor.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

void cursor1(jmp_buf buf) {
  const char* str
    = "excuse-moi, \n"
      "bonjour, \n"
      "bonsoir, \n"
      "rouge, hehe. :D \n";
  string_view sv = new_sv(str, strlen(str));
  Cursor c = new_cursor(sv);
  while (advance_cursor(&c) != '\n');

  // dump_cursor(&c);

  string_view result = get_till_newline_or_eof(&c);
  if (!s_eq(result, sv_from_cstr("bonjour, "))) {
    longjmp(buf, 1);
  }
}
