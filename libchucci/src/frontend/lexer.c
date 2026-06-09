#include "frontend/lexer.h"
#include "compiler.h"
#include "frontend/cursor.h"
#include "frontend/token.h"
#include "utils/diagnostics.h"
#include "utils/file.h"
#include "utils/string.h"
#include "utils/string_interner.h"
#include "utils/vmem_arena.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>

#define current_source(lexer, ctx)                                             \
  (filevec_access(&(ctx)->sources, lexer->file_pos))
#define current_source_ptr(lexer, ctx)                                         \
  (filevec_access_ptr(&(ctx)->sources, lexer->file_pos))
#define cursor(lexer) (&(lexer)->cursor)
#define get_cursor_mark(lexer) cursor_mark(cursor(lexer))

StringID keyword_to_id[__token_kind_count];

Lexer *lexer_new(CompilerCtx *ctx) {
#define X(kind, str)                                                           \
  keyword_to_id[kind] = intern(cstr_to_sv(str), ctx->interner);
  KEYWORDS(X)
#undef X
  Lexer *lexer = vmarena_calloc(ctx->arena, sizeof(Lexer));
  lexer->file_pos = 0;
  lexer->cursor = cursor_new(current_source(lexer, ctx));
  return lexer;
}

Token lex_op_sep(Lexer *lexer, CompilerCtx *ctx, char ch) {
#define X(kind, str, ch1)                                                      \
  if (ch1 == ch && cursor_match_str(cursor(lexer), cstr_to_sv(str)))           \
    return new_tok_simple(get_cursor_mark(lexer), const_cstr_to_sv(str),       \
                          current_source_ptr(lexer, ctx), kind);
  OPERATORS(X)
  SEPARATORS(X)
#undef X
  assert(false);
}

Token lex_num(Lexer *lexer, CompilerCtx *ctx) {
  CursorMark mark1 = get_cursor_mark(lexer);
  char ch = cursor_advance(cursor(lexer));
  ch = cursor_peek(cursor(lexer));
  bool is_float = false;
  while (true) {
    char _ch = cursor_peek(cursor(lexer));
    if (!isdigit(_ch) && _ch != '.')
      break;
    if (ch == '.') {
      if (is_float) {
        CursorMark mark2 = cursor_mark(cursor(lexer));
        Diagnostic diag = diagnostic_new(ERR_INVALID_NUMERIC_LITERAL,
                                         lexer->cursor, mark1, mark2);
        diagnostic_add(ctx->engine, diag);
      }
      is_float = true;
    }
    ch = cursor_advance(cursor(lexer));
  }
  CursorMark mark2 = get_cursor_mark(lexer);

  StringView lexeme = cursor_slice(cursor(lexer), mark1.id, mark2.id);
  return new_tok_val(mark1, lexeme, current_source_ptr(lexer, ctx));
}

Token lex_str(Lexer *lexer, CompilerCtx *ctx) {
  CursorMark mark1 = get_cursor_mark(lexer);
  char ch = cursor_advance(cursor(lexer));
  while (true) {
    ch = cursor_peek(cursor(lexer));
    if (ch == '\\') {
      cursor_advance(cursor(lexer)); // skip '\\'
      cursor_advance(cursor(lexer)); // skip the escape character
      ch = cursor_peek(cursor(lexer));
    }
    if (ch == '\"') {
      cursor_advance(cursor(lexer));
      break;
    }
    if (ch == '\0' || ch == '\n') {
      CursorMark mark2 = cursor_mark(cursor(lexer));
      Diagnostic diag =
          diagnostic_new(ERR_UNTERMINATED_STRING, lexer->cursor, mark1, mark2);
      diagnostic_add(ctx->engine, diag);
      break;
    }
    cursor_advance(cursor(lexer));
  }
  CursorMark mark2 = get_cursor_mark(lexer);

  StringView lexeme = cursor_slice(cursor(lexer), mark1.id, mark2.id);
  return new_tok_val(mark1, lexeme, current_source_ptr(lexer, ctx));
}

Token lex_ident(Lexer *lexer, CompilerCtx *ctx, char ch) {
  CursorMark mark1 = get_cursor_mark(lexer);
  while (true) {
    ch = cursor_peek(cursor(lexer));
    if (!isalnum((unsigned char)ch) && ch != '_')
      break;
    cursor_advance(cursor(lexer));
  }
  CursorMark mark2 = get_cursor_mark(lexer);

  StringView lexeme = cursor_slice(cursor(lexer), mark1.id, mark2.id);
  StringID id = intern(lexeme, ctx->interner);

#define X(kind, _)                                                             \
  if (keyword_to_id[kind] == id)                                               \
    return new_tok_simple(get_cursor_mark(lexer), lexeme,                      \
                          current_source_ptr(lexer, ctx), kind);
  KEYWORDS(X)
#undef X

  return new_tok_ident(mark1, lexeme, current_source_ptr(lexer, ctx), id);
}

void skip_comments(Lexer *lexer, CompilerCtx *ctx, char ch) {
  CursorMark mark1 = cursor_mark(cursor(lexer));
  cursor_advance(cursor(lexer));
  ch = cursor_peek(cursor(lexer));
  // single line comment
  if (ch == '/') {
    while (ch != '\0' && ch != '\n') {
      cursor_advance(cursor(lexer));
      ch = cursor_peek(cursor(lexer));
    }
    return;
  }
  // Multi line comments
  else if (ch == '*') {
    while (true) {
      if (ch == '\0') {
        CursorMark mark2 = cursor_mark(cursor(lexer));
        Diagnostic diag = diagnostic_new(ERR_UNTERMINATED_MULTILINE_COMMENT,
                                         lexer->cursor, mark1, mark2);
        diagnostic_add(ctx->engine, diag);
        break;
      }
      if (ch == '*') {
        cursor_advance(cursor(lexer));
        ch = cursor_peek(cursor(lexer));
        if (ch == '/') {
          cursor_advance(cursor(lexer));
          ch = cursor_peek(cursor(lexer));
          break;
        }
      }
      cursor_advance(cursor(lexer));
      ch = cursor_peek(cursor(lexer));
    }
  }
}

Token lex_next_token(Lexer *lexer, CompilerCtx *ctx) {
  skip_whitespace_except_newline(cursor(lexer));
  char ch = cursor_peek(cursor(lexer));
  if (ch == '/') {
    char next = cursor_peek_next(cursor(lexer));
    if (next == '*' || next == '/') {
      skip_comments(lexer, ctx, ch);
      return lex_next_token(lexer, ctx);
    }
  }
  if (ch == '\0') {
    if (ctx->sources.len > 1) {
      file_free(filevec_top_ptr(&ctx->sources));
      filevec_pop(&ctx->sources);
      lexer->cursor = cursor_new(current_source(lexer, ctx));
      return lex_next_token(lexer, ctx);
    }
    return new_tok_simple(
        get_cursor_mark(lexer),
        cursor_slice(cursor(lexer), lexer->cursor.id, lexer->cursor.id + 1),
        current_source_ptr(lexer, ctx), TOK_EOF);
  }
  if (ch == '\n') {
    cursor_advance(cursor(lexer));
    return new_tok_simple(
        get_cursor_mark(lexer),
        cursor_slice(cursor(lexer), lexer->cursor.id, lexer->cursor.id + 1),
        current_source_ptr(lexer, ctx), SEP_NEWLINE);
  }
  if (isalpha((unsigned char)ch) || ch == '_')
    return lex_ident(lexer, ctx, ch);
  if (ch == '\"')
    return lex_str(lexer, ctx);
  if (isdigit(ch))
    return lex_num(lexer, ctx);
  if (is_op(ch) || is_sep(ch))
    return lex_op_sep(lexer, ctx, ch);
  assert(false);
}
