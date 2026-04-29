#include <da_path.h>
#include <thirdparty/kvec.h>
#include <cursor.h>
#include <da_string.h>
#include <preprocess_1.h>


Preprocessor1 new_pp1(string_view source) {
  Preprocessor1 pp1 = {0};
  kv_init(pp1.include_dirs);
  pp1.cursor = new_cursor(source);
  pp1.builder = new_ds();
  return pp1;
}

void open_included_file(Preprocessor1* pp1, string_view file) {
  da_string builder = new_ds();
  for (size_t i=0; i<kv_size(pp1->include_dirs); i++) {
    string_view dir = kv_A(pp1->include_dirs, i);
    ds_push(&builder, &dir);
    ds_push_char(&builder, '/');
    ds_push(&builder, &file);
    Path path = new_path(ds_to_sv(&builder));
    
    if (path_exists(&path)) {
      string_view contents = read_file(&path);
      ds_push(&pp1->builder, &contents);
      free_sv(&contents);
      free_ds(&builder);
      free_path(&path);
      return;
    }
    ds_reset(&builder);
    free_path(&path);
  }
  free_ds(&builder);

  assert(false && "Included file not found");
}

void resolve_include(Preprocessor1* pp1) {
  skip_whitespace(&pp1->cursor);
  switch (peek(&pp1->cursor)) {
    case '\"': {
      ch_match_cursor(&pp1->cursor, '\"');
      string_view filename = get_till_delim(&pp1->cursor, '\"');
      open_included_file(pp1, filename);
      advance_cursor_by(&pp1->cursor, filename.len+1); // skip filename and the closing "
      break;
    }
    case '<': {
      ch_match_cursor(&pp1->cursor, '<');
      string_view filename = get_till_delim(&pp1->cursor, '>');
      open_included_file(pp1, filename);
      advance_cursor_by(&pp1->cursor, filename.len+1); // skip filename and closing >
      break;
    }
    default:
      dump_cursor(&pp1->cursor);
      assert(false && "Invalid character after #include");
  }
}

string_view resolve_pp1(Preprocessor1* pp1) {
  while (is_cursor_valid(&pp1->cursor)) {
    dump_cursor(&pp1->cursor);
    char current = peek(&pp1->cursor);
    switch (current) {
      case '#': {
        Cursor mark = mark_cursor(&pp1->cursor);
        ch_match_cursor(&pp1->cursor, '#');
        skip_whitespace(&pp1->cursor);
        if (str_match_cursor(&pp1->cursor, sv_from_cstr("include"))) {
          resolve_include(pp1);
          break;
        }
        rewind_cursor(&pp1->cursor, &mark);
        // fallthrough default: for #define etc....
      }
      default:
        ds_push_char(&pp1->builder, current);
        advance_cursor(&pp1->cursor);
    }
  }
  string_view str = ds_build(&pp1->builder);
  free_ds(&pp1->builder);
  return str;
}
