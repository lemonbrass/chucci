#include <cursor.h>
#include <da_string.h>
#include <preprocess_1.h>


Preprocessor1 new_pp1(string_view source, arena_t* arena) {
  Preprocessor1 pp1 = {0};
  pp1.cursor = new_cursor(source);
  pp1.builder = new_ds(arena);
  pp1.arena = arena;
  return pp1;
}

void resolve_include(Preprocessor1* pp1) {
  skip_whitespace(&pp1->cursor);
  switch (peek(&pp1->cursor)) {
    case '\"': {
      ch_match_cursor(&pp1->cursor, '\"');
      string_view filename = get_till_delim(&pp1->cursor, '\"');
      printf("Found #include \"%.*s\"\n", (int)filename.len, filename.str);
      assert(false && "TODO");
      break;
    }
    case '<': {
      ch_match_cursor(&pp1->cursor, '<');
      string_view filename = get_till_delim(&pp1->cursor, '>');
      printf("Found #include <%.*s>\n", (int)filename.len, filename.str);
      assert(false && "TODO");
      break;
    }
    default:
      dump_cursor(&pp1->cursor);
      assert(false && "Invalid character after #include");
  }
}

string_view resolve_pp1(Preprocessor1* pp1) {
  while (is_cursor_valid(&pp1->cursor)) {
    char current = peek(&pp1->cursor);
    dump_cursor(&pp1->cursor);
    switch (current) {
      case '#': {
        ch_match_cursor(&pp1->cursor, '#');
        skip_whitespace(&pp1->cursor);
        if (str_match_cursor(&pp1->cursor, sv_from_cstr("include"))) {
          resolve_include(pp1);
          break;
        }
      }
      default:
        ds_push_char(&pp1->builder, current);
    }
    advance_cursor(&pp1->cursor);
  }
  return ds_build(&pp1->builder);
}
