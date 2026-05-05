#ifndef LEXER_H
#define LEXER_H

#include <token.h>
#include <compiler.h>
#include <cursor.h>

typedef CursorMark LexerMark;

typedef struct {
  Cursor cursor;
  ChucciCompiler* ctx;
} Lexer;

Lexer new_lexer(ChucciCompiler* ctx);
LexerMark mark_lexer(Lexer* lexer);
void rewind_lexer(Lexer* lexer, LexerMark* mark);

Token lex_next_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);
Token lexer_peek_nth(Lexer* lexer, size_t n);
Token lexer_expect_token_kind(Lexer* lexer, TokenKind kind);
void lexer_throw_error(Lexer* lexer, Token errtok, const char* errormsg);

#endif
