#ifndef __TOKEN_STREAM_H
#define __TOKEN_STREAM_H

#include "compiler.h"
#include "frontend/token.h"
#include "utils/smallvec.h"

VEC_DEF(Token, TokenVec, tokenvec, VMEM_ARENA_ALLOC_INT)

typedef enum TokenStreamKind {
  TS_VEC,
  TS_LEXER,
  TS_MACRO_USE,
} TokenStreamKind;

typedef struct MacroUseStream MacroUseStream;
typedef struct TokenStream {
  TokenStreamKind kind;
  bool is_consumed;
  union {
    Lexer *lexer;
    struct {
      TokenVec vec;
      size_t pos;
    };
    MacroUseStream *macro_use;
  };
} TokenStream;

SMALLVEC_DEF(TokenStream, TokenStreamStack, ts_stack, VMEM_ARENA_ALLOC_INT);

TokenStream ts_from_lexer(Lexer *lexer);
TokenStream ts_from_vec(TokenVec vec);
TokenStream ts_from_macro_use(MacroUseStream *macro_use);
Token ts_next_token(TokenStream *ts, CompilerCtx *ctx);
Token ts_peek_token(TokenStream *ts, CompilerCtx *ctx);
Token ts_expect_token(TokenStream *ts, CompilerCtx *ctx, TokenKind kind);
Token ts_stack_expect_token(TokenStreamStack *stack, CompilerCtx *ctx, TokenKind kind);
Token ts_stack_peek_token(TokenStreamStack *stack, CompilerCtx *ctx);
Token ts_stack_next_token(TokenStreamStack *stack, CompilerCtx *ctx);
void ts_free(TokenStream *ts, CompilerCtx *ctx);

#endif
