#ifndef LEXER_H
#define LEXER_H

#include <ctx.h>
#include <cursor.h>

typedef CursorMark LexerMark;

typedef struct {
  Cursor cursor;
  CompilerCtx ctx;
} Lexer;

Lexer new_lexer(string_view source);
void lex_next_token(string_view source);
Lexer peek_next_token(string_view source);


#endif
