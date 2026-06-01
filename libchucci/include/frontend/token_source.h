#ifndef TOKEN_SOURCE_H
#define TOKEN_SOURCE_H

#include <compiler.h>
#include <utils/da_string.h>
#include <frontend/token.h>
#include <thirdparty/kvec.h>
#include <frontend/lexer.h>

typedef enum SourceKind {
  SK_ARRAY,
  SK_LEXER,
} SourceKind;

typedef struct {
  union {
    Lexer* lexer;
    struct {
      TokenArray tokens;
      size_t pos;
    } array;
  };
  SourceKind kind;  
} TokenSource;

TokenSource ts_from_lexer(Lexer* lexer);
TokenSource ts_from_array(TokenArray array);

Token next_token(TokenSource* src);
Token peek_token(TokenSource* src);
Token peek_nth(TokenSource* src, size_t n);
Token expect_token_kind(TokenSource* src, TokenKind kind, ChucciCompiler* ctx);

#define throw_error(src, errtok, errmsg, ctx) _throw_error(src, errtok, errmsg, ctx, __FILE__, __LINE__)
#define new_tokenarray() (TokenArray){0}

void print_token_array(TokenArray* array);
_Noreturn void _throw_error(TokenSource* src, Token errtok, const char* errmsg, ChucciCompiler* ctx, const char* file, int line);
void give_warning(TokenSource* src, Token warningtok, const char* warningmsg, ChucciCompiler* ctx);

#endif
