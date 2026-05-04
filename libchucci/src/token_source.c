#include <cursor.h>
#include <da_string.h>
#include <lexer.h>
#include <token.h>
#include <assert.h>
#include <token_source.h>


TokenSource ts_from_lexer(Lexer* lexer) {
  TokenSource src = {0};
  src.kind = SK_LEXER;
  src.lexer = lexer;
  return src;
}
TokenSource ts_from_array(TokenArray array, string_view source) {
  TokenSource src = {0};
  src.kind = SK_ARRAY;
  src.array.tokens = array;
  src.array.pos = 0;
  src.array.source = source;
  return src;
}

Token next_token(TokenSource* src) {
  switch (src->kind) {
    case SK_ARRAY:
      return kv_A(src->array.tokens, src->array.pos++);
    case SK_LEXER:
      return lex_next_token(src->lexer);
  }
}

Token peek_token(TokenSource* src) {
  switch (src->kind) {
    case SK_ARRAY:
      return kv_A(src->array.tokens, src->array.pos);
    case SK_LEXER:
      return lexer_peek_token(src->lexer);
  }
}

Token peek_nth(TokenSource* src, size_t n) {
  switch (src->kind) {
    case SK_ARRAY:
      return kv_A(src->array.tokens, src->array.pos + n);
    case SK_LEXER:
      return lexer_peek_nth(src->lexer, n);
  }
}

Token expect_token_kind(TokenSource* src, TokenKind kind) {
  Token token = next_token(src);
  assert(token.kind == kind);
  return token;
}

void throw_error(TokenSource* src, Token errtok, const char* errormsg) {
  printf("Error: %s: ", errormsg);
  print_token_pretty(&errtok);
  printf("\n");
  Cursor cursor;
  cursor.line = errtok.pos.line;
  cursor.id = errtok.pos.id;
  cursor.col = errtok.pos.col;
  cursor.source = (src->kind==SK_LEXER) ? src->lexer->cursor.source : src->array.source;
  dump_cursor(&cursor);
  assert(false);
}
