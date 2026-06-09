#include "frontend/token_stream.h"
#include "compiler.h"
#include "frontend/lexer.h"
#include "frontend/token.h"
#include "utils/chucci_alloc.h"
#include "utils/smallvec.h"

SMALLVEC_IMPL(TokenStream, TokenStreamStack, ts_stack, VMEM_ARENA_ALLOC_INT)
SMALLVEC_IMPL(Token, TokenVec, tokenvec, VMEM_ARENA_ALLOC_INT)

Token ts_stack_next_token(TokenStreamStack *stack, CompilerCtx *ctx) {
  assert(stack->len > 0);
  Token token = ts_next_token(ts_stack_top_ptr(stack), ctx);
  if (token.kind == TOK_EOF && stack->len > 1) {
    TokenStream ts = ts_stack_pop(stack);
    ts_free(&ts, ctx);
    token = ts_stack_next_token(stack, ctx);
  }
  return token;
}

TokenStream ts_from_lexer(Lexer *lexer) {
  TokenStream ts = {0};
  ts.is_consumed = false;
  ts.kind = TS_LEXER;
  ts.lexer = lexer;
  return ts;
}
TokenStream ts_from_vec(TokenVec vec) {
  TokenStream ts = {0};
  ts.is_consumed = false;
  ts.kind = TS_VEC;
  ts.vec = vec;
  ts.pos = 0;
  return ts;
}

Token ts_next_token(TokenStream *ts, CompilerCtx *ctx) {
  if (ts->kind == TS_LEXER) {
    Token token = lex_next_token(ts->lexer, ctx);
    if (token.kind == TOK_EOF)
      ts->is_consumed = true;
    return token;
  } else {
    assert(ts->pos < ts->vec.len);
    Token token = tokenvec_access(&ts->vec, ts->pos++);
    if (token.kind == TOK_EOF)
      ts->is_consumed = true;
    return token;
  }
}

void ts_free(TokenStream *ts, CompilerCtx *ctx) {
  if (ts->kind == TS_VEC)
    tokenvec_free(&ts->vec, ctx);
}
