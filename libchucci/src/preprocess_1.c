#include <ctx.h>
#include <da_path.h>
#include <thirdparty/kvec.h>
#include <cursor.h>
#include <da_string.h>
#include <preprocess_1.h>
#include <da_assert.h>

Preprocessor1 new_pp1(string_view source, CompilerCtx* ctx) {
  Preprocessor1 pp1 = {0};
  pp1.cursor = new_cursor(source);
  pp1.ctx = ctx;
  return pp1;
}

void check_cyclic_includes(Preprocessor1* pp1, Path path) {
  for (size_t i = 0; i < kv_size(pp1->ctx->included_files); i++) {
    Path path2 = kv_A(pp1->ctx->included_files, i);
    da_assert(!path_cmp(&path, &path2) && "Cyclic include detected");
  }
}

void open_included_file(Preprocessor1* pp1, string_view file) {
  da_string builder = new_ds();
  for (size_t i=0; i<kv_size(pp1->ctx->options->include_dirs); i++) {
    string dir = kv_A(pp1->ctx->options->include_dirs, i);
    push_ds(&builder, str_to_sv(dir));
    push_char_ds(&builder, '/');
    push_ds(&builder, file);

    Path path = new_path(ds_to_sv(&builder));
    
    if (path_exists(&path)) {
      check_cyclic_includes(pp1, path);
      kv_push(Path, pp1->ctx->included_files, path);
      string contents = read_file(&path);
      Preprocessor1 child = new_pp1(str_to_sv(contents), pp1->ctx);
      string processed = resolve_pp1(&child);
      push_ds(&pp1->ctx->buf, str_to_sv(processed));
      free_str(&contents);
      free_str(&processed);
      free_ds(&builder);
      return;
    }
    reset_ds(&builder);
    free_path(&path);
  }
  free_ds(&builder);

  da_assert(false && "Included file not found");
}

void resolve_include(Preprocessor1* pp1) {
  skip_whitespace_except_newline(&pp1->cursor);
  string_view filename = {0};
  switch (peek(&pp1->cursor)) {
    case '\"': {
      ch_match_cursor(&pp1->cursor, '\"');
      filename = get_till_delim(&pp1->cursor, '\"');
      break;
    }
    case '<': {
      ch_match_cursor(&pp1->cursor, '<');
      filename = get_till_delim(&pp1->cursor, '>');
      break;
    }
    default:
      dump_cursor(&pp1->cursor);
      da_assert(false && "Invalid character after #include");
  }
  open_included_file(pp1, filename);
  advance_cursor_by(&pp1->cursor, filename.len+1); // skip filename and the closing "
  skip_whitespace_except_newline(&pp1->cursor);
  if (!ch_match_cursor(&pp1->cursor, '\n') && !ch_match_cursor(&pp1->cursor, '\0')) da_assert(false && "expected newline or eof after #include");
}

string resolve_pp1(Preprocessor1* pp1) {
  while (is_cursor_valid(&pp1->cursor)) {
    char current = peek(&pp1->cursor);
    switch (current) {
      case '#': {
        CursorMark mark = mark_cursor(&pp1->cursor);
        ch_match_cursor(&pp1->cursor, '#');
        skip_whitespace(&pp1->cursor);
        if (str_match_cursor(&pp1->cursor, sv_from_cstr("include"))) {
          resolve_include(pp1);
          break;
        }
        rewind_cursor(&pp1->cursor, &mark);

        goto default_body;
        break;
      }
      case '\\': {
        if  (ch_match_cursor(&pp1->cursor, '\n')) break;
        goto default_body;
      }
      case '/': {
        // line comments
        CursorMark mark = mark_cursor(&pp1->cursor);
        ch_expect_cursor(&pp1->cursor, '/');
        if (ch_match_cursor(&pp1->cursor, '/')) {
          while (peek(&pp1->cursor) != '\n' && peek(&pp1->cursor) != '\0') {
            if (peek(&pp1->cursor) == '\\' && peek_next(&pp1->cursor) == '\n') {
              ch_expect_cursor(&pp1->cursor, '\\');
              ch_expect_cursor(&pp1->cursor, '\n');
            }
            advance_cursor(&pp1->cursor);
          }
          break;
        }
        rewind_cursor(&pp1->cursor, &mark);

        // block comments
        ch_expect_cursor(&pp1->cursor, '/');
        if (ch_match_cursor(&pp1->cursor, '*')) {
          while (peek(&pp1->cursor) != '*' || peek_next(&pp1->cursor) != '/') {
            da_assert(peek_next(&pp1->cursor) != '\0' && "Unexpected EOF in block comment");
            advance_cursor(&pp1->cursor);
          }
          ch_expect_cursor(&pp1->cursor, '*');
          ch_expect_cursor(&pp1->cursor, '/');
          break;
        }
        rewind_cursor(&pp1->cursor, &mark);

        goto default_body;
        break;
      }
      default: {
        default_body:
          push_char_ds(&pp1->ctx->buf, current);
          advance_cursor(&pp1->cursor);
      }
    }
  }
  string str = build_ds(&pp1->ctx->buf);
  reset_ds(&pp1->ctx->buf);
  return str;
}
