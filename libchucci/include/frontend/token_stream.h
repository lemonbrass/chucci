#ifndef __TOKEN_STREAM_H
#define __TOKEN_STREAM_H

#include "compiler.h"
#include "frontend/token.h"
#include "utils/smallvec.h"

SMALLVEC_DEF(Token, TokenVec, tokenvec)

typedef enum TokenStreamKind {
  TS_VEC,
  TS_LEXER,
} TokenStreamKind;

typedef struct TokenStream {
  TokenStreamKind kind;
  bool is_consumed;
  union {
    Lexer *lexer;
    struct {
      TokenVec vec;
      size_t pos;
    };
  };
} TokenStream;

SMALLVEC_DEF(TokenStream, TokenStreamStack, ts_stack);

TokenStream ts_from_lexer(Lexer *lexer);
TokenStream ts_from_vec(TokenVec vec);
Token ts_next_token(TokenStream *ts, CompilerCtx *ctx);
Token ts_stack_next_token(TokenStreamStack *stack, CompilerCtx *ctx);
void ts_free(TokenStream *ts, CompilerCtx *ctx);

#endif
