#ifndef __TOKEN_STREAM_H
#define __TOKEN_STREAM_H

#include "compiler.h"
#include "frontend/token.h"
#include "utils/smallvec.h"

VEC_DEF(Token, TokenVec, tokenvec, VMEM_ARENA_ALLOC_INT)

typedef enum TokenStreamKind {
  TS_VEC,
  TS_LEXER,
  TS_PREPROCESSOR,
  TS_MACRO_USE,
  TS_SINGLE, // single token
} TokenStreamKind;

typedef struct MacroUseStream MacroUseStream;
typedef struct TokenStream {
  TokenStreamKind kind;
  bool is_consumed;
  union {
    Lexer *lexer;
    Preprocessor *pp;
    struct {
      TokenVec vec;
      size_t pos;
    };
    Token single;
    MacroUseStream *macro_use;
  };
} TokenStream;

SMALLVEC_DEF_WITH_FIELDS(TokenStream, TokenStreamStack, ts_stack, VMEM_ARENA_ALLOC_INT, bool is_consumed;);

TokenStream ts_from_lexer(Lexer *lexer);
TokenStream ts_from_preprocessor(Preprocessor *pp);
TokenStream ts_from_vec(TokenVec vec);
TokenStream ts_from_macro_use(MacroUseStream *macro_use);
TokenStream ts_from_token(Token token);
Token ts_next_token(TokenStream *ts, CompilerCtx *ctx);
Token ts_peek_token(TokenStream *ts, CompilerCtx *ctx);
Token ts_expect_token(TokenStream *ts, CompilerCtx *ctx, TokenKind kind);
Token ts_stack_expect_token(TokenStreamStack *stack, CompilerCtx *ctx, TokenKind kind);
Token ts_stack_peek_token(TokenStreamStack *stack, CompilerCtx *ctx);
Token ts_stack_next_token(TokenStreamStack *stack, CompilerCtx *ctx);
void ts_free(TokenStream *ts, CompilerCtx *ctx);

void unexpected_token_err(CompilerCtx *ctx, Token unexpected, TokenKind expected);

#endif
