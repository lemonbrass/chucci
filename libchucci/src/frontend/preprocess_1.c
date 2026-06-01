#include <compiler.h>
#include <frontend/cursor.h>
#include <frontend/preprocess_1.h>
#include <stdio.h>
#include <thirdparty/kvec.h>
#include <utils/bufferpool.h>
#include <utils/da_path.h>
#include <utils/da_string.h>

#define options ctx->global->options
#define strings ctx->global->strings
#define pp1cursor &pp1->cursor

Preprocessor1 new_pp1(PPCtx *ctx) {
  Preprocessor1 pp1 = {0};
  pp1.cursor = new_cursor(str_to_sv(kv_top(options->source_stack)));
  pp1.ctx = ctx;
  return pp1;
}

bool check_cyclic_includes(Preprocessor1 *pp1, Path path) {
  for (size_t i = 0; i < kv_size(pp1->ctx->included_files); i++) {
    Path path2 = kv_A(pp1->ctx->included_files, i);
    if (path_eq(&path, &path2)) {
      return true;
    }
  }
  return false;
}

Path get_included_path(string dir, da_string *path_builder, string_view file) {
  push_ds(path_builder, str_to_sv(dir));
  push_char_ds(path_builder, '/');
  push_ds(path_builder, file);
  push_char_ds(path_builder, '\0');
  return new_path_from_cstr_borrowed(get_cstr_from_ds(path_builder));
}

void open_included_file(Preprocessor1 *pp1, string_view file,
                        da_string *preprocessed) {
  da_string *path_builder = bp_get(pp1->strings, new_ds);
  for (size_t i = 0; i < kv_size(pp1->options->include_dirs); i++) {
    string dir = kv_A(pp1->options->include_dirs, i);
    Path path = get_included_path(dir, path_builder, file);

    if (path_exists(&path)) {
      if (check_cyclic_includes(pp1, path)) {
        printf("Error: Cyclic includes detected\n");
        dump_cursor(pp1cursor);
        initiate_error(pp1->ctx->global);
      }
      string contents = read_file(&path);

      kv_push(Path, pp1->ctx->included_files, path);
      kv_push(string, pp1->options->source_stack, contents);
      Preprocessor1 child = new_pp1(pp1->ctx);
      string processed_contents = resolve_pp1(&child);
      kv_pop(pp1->ctx->included_files);
      kv_pop(pp1->options->source_stack);

      push_ds(preprocessed, str_to_sv(processed_contents));

      free_str(&contents);
      free_str(&processed_contents);
      return;
    }

    reset_ds(path_builder);
  }
  bp_save(pp1->strings, path_builder, reset_ds);

  printf("Error: Included file not found\n");
  dump_cursor(pp1cursor);
  initiate_error(pp1->ctx->global);
}

void resolve_include(Preprocessor1 *pp1, da_string *preprocessed) {
  skip_whitespace_except_newline(pp1cursor);
  string_view filename = {0};
  switch (peek(pp1cursor)) {
  case '\"': {
    ch_match_cursor(pp1cursor, '\"');
    filename = get_till_delim(pp1cursor, '\"');
    break;
  }
  case '<': {
    ch_match_cursor(pp1cursor, '<');
    filename = get_till_delim(pp1cursor, '>');
    break;
  }
  default:
    printf("Error: Invalid character after #include\n");
    dump_cursor(pp1cursor);
    initiate_error(pp1->ctx->global);
  }
  open_included_file(pp1, filename, preprocessed);
  advance_cursor_by(pp1cursor,
                    filename.len + 1); // skip filename and the closing "
  skip_whitespace_except_newline(pp1cursor);
  if (!ch_match_cursor(pp1cursor, '\n') && !ch_match_cursor(pp1cursor, '\0')) {
    printf("Error: Expected newline or eof after #include\n");
    initiate_error(pp1->ctx->global);
  }
}

string resolve_pp1(Preprocessor1 *pp1) {
  da_string *preprocessed = bp_get(pp1->strings, new_ds);
  while (peek(pp1cursor) != '\0') {
    // dump_cursor(pp1cursor);
    char current = peek(pp1cursor);
    switch (current) {
    case '#': {
      CursorMark mark = mark_cursor(pp1cursor);
      ch_match_cursor(pp1cursor, '#');
      skip_whitespace(pp1cursor);
      if (str_match_cursor(pp1cursor, sv_from_cstr("include"))) {
        resolve_include(pp1, preprocessed);
        break;
      }
      rewind_cursor(pp1cursor, &mark);

      goto default_body;
      break;
    }
    case '\\': {
      if (ch_match_cursor(pp1cursor, '\n'))
        break;
      goto default_body;
    }
    case '/': {
      // line comments
      CursorMark mark = mark_cursor(pp1cursor);
      ch_expect_cursor(pp1cursor, '/');
      if (ch_match_cursor(pp1cursor, '/')) {
        while (peek(pp1cursor) != '\n' && peek(pp1cursor) != '\0') {
          if (peek(pp1cursor) == '\\' && peek_next(pp1cursor) == '\n') {
            ch_expect_cursor(pp1cursor, '\\');
            ch_expect_cursor(pp1cursor, '\n');
          }
          advance_cursor(pp1cursor);
        }
        push_char_ds(preprocessed, ' ');
        break;
      }
      rewind_cursor(pp1cursor, &mark);

      // block comments
      ch_expect_cursor(pp1cursor, '/');
      if (ch_match_cursor(pp1cursor, '*')) {
        while (peek(pp1cursor) != '\0') {
          if (peek(pp1cursor) == '*' && peek_next(pp1cursor) == '/') {
            break;
          }
          advance_cursor(pp1cursor);
        }
        if (peek(pp1cursor) == '\0') {
          printf("Error: Unexpected EOF in block comment\n");
          dump_cursor(pp1cursor);
          initiate_error(pp1->ctx->global);
        }
        ch_expect_cursor(pp1cursor, '*');
        ch_expect_cursor(pp1cursor, '/');
        push_char_ds(preprocessed, ' ');
        break;
      }
      rewind_cursor(pp1cursor, &mark);

      goto default_body;
      break;
    }
    default: {
    default_body:
      push_char_ds(preprocessed, current);
      advance_cursor(pp1cursor);
    }
    }
  }
  string str = build_ds(preprocessed);
  bp_save(pp1->strings, preprocessed, reset_ds);
  return str;
}
