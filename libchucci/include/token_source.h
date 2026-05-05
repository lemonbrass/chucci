#ifndef TOKEN_SOURCE_H
#define TOKEN_SOURCE_H

#include <ctx.h>
#include <da_string.h>
#include <token.h>
#include <thirdparty/kvec.h>
#include <lexer.h>

typedef kvec_t(Token) TokenArray;

typedef enum SourceKind {
  SK_ARRAY,
  SK_LEXER,
} SourceKind;

typedef struct {
  union {
    Lexer* lexer;
    struct {
      TokenArray tokens;
      string_view source;
      size_t pos;
    } array;
  };
  SourceKind kind;  
} TokenSource;

TokenSource ts_from_lexer(Lexer* lexer);
TokenSource ts_from_array(TokenArray array, string_view source);

Token next_token(TokenSource* src);
Token peek_token(TokenSource* src);
Token peek_nth(TokenSource* src, size_t n);
Token expect_token_kind(TokenSource* src, TokenKind kind, CompilerCtx* ctx);

void print_token_array(TokenArray* array);
void throw_error(TokenSource* src, Token errtok, const char* errmsg, CompilerCtx* ctx);

#endif
