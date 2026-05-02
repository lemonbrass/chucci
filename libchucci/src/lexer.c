#include <da_intern.h>
#include <da_string.h>
#include <token.h>
#include <cursor.h>
#include <ctx.h>
#include <lexer.h>


Lexer new_lexer(string_view source, CompilerCtx* ctx) {
  Lexer lexer;
  lexer.ctx = ctx;
  lexer.cursor = new_cursor(source);
  return lexer;
}

Token lex_ident(Lexer* lexer) {
  CursorMark pos = mark_cursor(&lexer->cursor);
  advance_cursor(&lexer->cursor);
  char ch;
  while ((ch = peek(&lexer->cursor)), is_alpha(ch) || ch == '_' || is_num(ch)) {
    advance_cursor(&lexer->cursor);
  }
  interned_str ident = intern(lexer->ctx->table, sv_slice(lexer->cursor.source, pos.id, lexer->cursor.id-pos.id));

  #define X(a, b)\
  if (interned_eq(ident, lexer->ctx->keywords[a])) return new_token(pos, a);
  KEYWORDS(X)
  #undef X

  return new_token(pos, ident);
}

// TODO: All other types of num lexing
Token lex_num(Lexer* lexer) {
  CursorMark pos = mark_cursor(&lexer->cursor);
  advance_cursor(&lexer->cursor);
  bool has_decimal = false;
  while (true) {
    char ch = peek(&lexer->cursor);
    if (is_num(ch)) {
      advance_cursor(&lexer->cursor);
    } else if (ch == '.' && is_num(peek_next(&lexer->cursor)) && !has_decimal) {
      has_decimal = true;
      advance_cursor(&lexer->cursor);
    } else {
      break;
    }
  }
  string_view num = sv_slice(lexer->cursor.source, pos.id, lexer->cursor.id-pos.id);
  return new_token(pos, num);
}

Token lex_op(Lexer* lexer) {
  CursorMark pos = mark_cursor(&lexer->cursor);

  #define X(a, b, c) if (str_match_cursor(&lexer->cursor, sv_from_cstr(b))) return new_token(pos, a);
  OPERATORS(X)
  #undef X

  dump_cursor(&lexer->cursor);
  assert(false && "UNREACHABLE");
}

Token lex_next_token(Lexer* lexer) {
  skip_whitespace(&lexer->cursor);
  char current = peek(&lexer->cursor);
  CursorMark pos = mark_cursor(&lexer->cursor);
  Token token = ERROR_TOKEN(pos, "Unknown token");

  if (is_alpha(current) || current == '_') {
    token = lex_ident(lexer);
    goto end;
  }

  if (is_num(current)) {
    token = lex_num(lexer);
    goto end;
  }

  if (is_op(current)) {
    token = lex_op(lexer);
    goto end;
  }

  // Separators and EOF
  switch (current) {
    case '\0':
      token = EOF_TOKEN(pos);
      advance_cursor(&lexer->cursor);
      goto end;
    #define X(a, b, c) \
    case c: token = new_token(pos, a);\
            advance_cursor(&lexer->cursor);\
            goto end;
    SEPARATORS(X)
    #undef X
  }

  // This line will be unreachable, because we goto end at every
  // valid token, hence, we advance if we got an invalid token
  advance_cursor(&lexer->cursor);

  end:
  return token;
}

