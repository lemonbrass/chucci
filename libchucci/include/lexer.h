#ifndef LEXER_H
#define LEXER_H

#include <token.h>
#include <ctx.h>
#include <cursor.h>

typedef CursorMark LexerMark;

typedef struct {
  Cursor cursor;
  CompilerCtx* ctx;
} Lexer;

Lexer new_lexer(string_view source, CompilerCtx* ctx);
LexerMark mark_lexer(Lexer* lexer);
void rewind_lexer(Lexer* lexer, LexerMark* mark);
Token lex_next_token(Lexer* lexer);
Token peek_next_token(Lexer* lexer);
Token peek_nth_token(Lexer* lexer, size_t n);

Token expect_token_kind(Lexer* lexer, TokenKind kind);
void throw_lexer_error(Lexer* lexer, Token errtok, const char* errormsg);

#endif
