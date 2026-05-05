#include <da_intern.h>
#include <da_string.h>
#include <setjmp.h>
#include <stdio.h>
#include <token.h>
#include <cursor.h>
#include <compiler.h>
#include <lexer.h>


Lexer new_lexer(ChucciCompiler* ctx) {
  Lexer lexer;
  lexer.ctx = ctx;
  lexer.cursor = new_cursor(str_to_sv(kv_top(ctx->source_stack)));
  return lexer;
}

Token lex_ident(Lexer* lexer) {
  Cursor pos = lexer->cursor;
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
  Cursor pos = lexer->cursor;
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
  Cursor pos = lexer->cursor;

  #define X(a, b, c) if (str_match_cursor(&lexer->cursor, sv_from_cstr(b))) return new_token(pos, a);
  OPERATORS(X)
  #undef X

  dump_cursor(&lexer->cursor);
  assert(false && "UNREACHABLE");
}

Token lex_str(Lexer* lexer) {
  Cursor pos = lexer->cursor;
  advance_cursor(&lexer->cursor);
  char ch;
  while ((ch = peek(&lexer->cursor), ch != '\"')) {
    if (ch == '\\') advance_cursor(&lexer->cursor);
    advance_cursor(&lexer->cursor);
  }
  advance_cursor(&lexer->cursor); // skip final "
  string_view str = sv_slice(lexer->cursor.source, pos.id, lexer->cursor.id-pos.id);
  return new_token(pos, str);
}

Token lex_next_token(Lexer* lexer) {
  skip_whitespace_except_newline(&lexer->cursor);
  char current = peek(&lexer->cursor);
  Cursor pos = lexer->cursor;
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

  if (current == '\"') {
    token = lex_str(lexer);
    goto end;
  }

  // Separators and EOF
  switch (current) {
    case '\0':
      token = EOF_TOKEN(pos);
      advance_cursor(&lexer->cursor);
      goto end;
    case '\n':
      token = new_token(pos, SEP_NEWLINE);
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

LexerMark mark_lexer(Lexer* lexer) {
  return mark_cursor(&lexer->cursor);
}

void rewind_lexer(Lexer* lexer, LexerMark* mark) {
  rewind_cursor(&lexer->cursor, mark);
}

Token lexer_peek_token(Lexer* lexer) {
  LexerMark mark = mark_lexer(lexer);
  Token token = lex_next_token(lexer);
  rewind_lexer(lexer, &mark);
  return token;
}

// TODO: A Token buffer in ctx, for caching lex_next_token in case of peek
Token lexer_peek_nth(Lexer* lexer, size_t n) {
  LexerMark mark = mark_lexer(lexer);
  Token token = lex_next_token(lexer);
  while (--n > 0 && token.kind != TOK_EOF) token = lex_next_token(lexer);
  rewind_lexer(lexer, &mark);
  return token;
}

Token lexer_expect_token_kind(Lexer* lexer, TokenKind kind) {
  Token token = lex_next_token(lexer);
  if (token.kind != kind) {
    printf("Error: expected %s, got ", tok_to_str[kind]);
    print_token_pretty(&token);
    printf("\n");
    dump_cursor(&token.pos);

    longjmp(*lexer->ctx->onerror, 1);
  }
  return token;
}

void lexer_throw_error(Lexer* lexer, Token errtok, const char* errormsg) {
  printf("Error: %s: ", errormsg);
  print_token_pretty(&errtok);
  printf("\n");
  dump_cursor(&errtok.pos);
  assert(false);
}
