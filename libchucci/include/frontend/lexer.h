#ifndef __LEXER_H
#define __LEXER_H

#include "compiler.h"
#include "frontend/cursor.h"
#include "frontend/token.h"

struct Lexer {
  Cursor cursor;
  CompilerCtx *ctx;
};

Lexer *lexer_new(CompilerCtx *ctx);
Token next_token(Lexer *lexer);


#endif
