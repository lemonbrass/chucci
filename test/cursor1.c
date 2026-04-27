#include <da_string.h>
#include <cursor.h>
#include <setjmp.h>
#include <string.h>

void cursor1(jmp_buf buf) {
  const char* str
    = "excuse-moi, \n"
      "bonjour, \n"
      "bonsoir, \n"
      "rouge, hehe. :D \n";
  string_view sv = sv_new(str, strlen(str));
  Cursor c = new_cursor(sv);
  while (advance_cursor(&c) != '\n');

  string_view result = get_full_line(&c);
  if (!s_cmp(result, sv_from_cstr("bonjour, "))) {
    longjmp(buf, 1);
  }
}
