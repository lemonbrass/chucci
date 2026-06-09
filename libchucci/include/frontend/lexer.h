#ifndef __LEXER_H
#define __LEXER_H

#include "compiler.h"
#include "frontend/cursor.h"
#include "frontend/token.h"

struct Lexer {
  Cursor cursor;
};

Lexer *lexer_new(CompilerCtx *ctx);
Token lex_next_token(Lexer *lexer, CompilerCtx *ctx);


#endif
