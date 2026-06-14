#include "frontend/token_stream.h"
#include "compiler.h"
#include "frontend/lexer.h"
#include "frontend/pp_macro.h"
#include "frontend/token.h"
#include "utils/chucci_alloc.h"
#include "utils/diagnostics.h"
#include "utils/smallvec.h"
#include "utils/string.h"
#include <assert.h>
#include <stdio.h>

SMALLVEC_IMPL(TokenStream, TokenStreamStack, ts_stack, VMEM_ARENA_ALLOC_INT)
VEC_IMPL(Token, TokenVec, tokenvec, VMEM_ARENA_ALLOC_INT)
LINEAR_MAP_IMPL(StringID, TokenVec, MacroArgMap, macroargmap,
                VMEM_ARENA_ALLOC_INT)

Token ts_stack_next_token(TokenStreamStack *stack, CompilerCtx *ctx) {
  assert(stack->len > 0);
  TokenStream *ts = ts_stack_top_ptr(stack);
  Token token = ts_next_token(ts, ctx);
  if (ts->is_consumed && stack->len > 1) {
    TokenStream ts = ts_stack_pop(stack);
    ts_free(&ts, ctx);
    return token;
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

TokenStream ts_from_macro_use(MacroUseStream *macro_use) {
  TokenStream ts = {0};
  ts.is_consumed = false;
  ts.kind = TS_MACRO_USE;
  ts.macro_use = macro_use;
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
  } else if (ts->kind == TS_VEC) {
    Token token = tokenvec_access(&ts->vec, ts->pos++);
    if (token.kind == TOK_EOF)
      ts->is_consumed = true;
    if (ts->pos >= ts->vec.len)
      ts->is_consumed = true;
    return token;
  } else if (ts->kind == TS_MACRO_USE) {
    return mu_next_token(ts, ctx);
  }
  assert(false);
}

void ts_free(TokenStream *ts, CompilerCtx *ctx) {
  if (ts->kind == TS_VEC)
    tokenvec_free(&ts->vec, ctx);
}

Token ts_peek_token(TokenStream *ts, CompilerCtx *ctx) {
  if (ts->kind == TS_LEXER) {
    Token token = lex_peek_token(ts->lexer, ctx);
    return token;
  } else if (ts->kind == TS_VEC) {
    assert(ts->pos < ts->vec.len);
    Token token = tokenvec_access(&ts->vec, ts->pos);
    return token;
  } else if (ts->kind == TS_MACRO_USE) {
    return mu_peek_token(ts, ctx);
  }
  assert(false);
}

Token ts_stack_peek_token(TokenStreamStack *stack, CompilerCtx *ctx) {
  assert(stack->len > 0);
  TokenStream *ts = ts_stack_top_ptr(stack);
  Token token = ts_peek_token(ts, ctx);
  size_t i = 0;
  while (ts->is_consumed && stack->len - i > 1) {
    ts = ts_stack_access_ptr(stack, stack->len - (++i) - 1);
  }
  token = ts_peek_token(ts, ctx);
  return token;
}

Token ts_expect_token(TokenStream *ts, CompilerCtx *ctx, TokenKind kind) {
  Token token = ts_next_token(ts, ctx);
  if (token.kind != kind) {
    DIAGID diag = diagnostic_new(ctx->engine, token.span, ERR_UNEXPECTED_TOKEN);
    DIAGID note =
        subdiagnostic_new(diag, ctx->engine, token.span, NOTE_EXPECTED_TOKEN);
    diag_set_format(ctx->engine, note,
                    "%[file](%[row]:%[col]): %[level]: %[msg] %[0]");
    diag_add_arg(ctx->engine, note,
                 diagarg_sv(cstr_to_sv((char *)tok_to_str[kind])));
  }
  return token;
}

Token ts_stack_expect_token(TokenStreamStack *stack, CompilerCtx *ctx,
                            TokenKind kind) {
  Token token = ts_stack_next_token(stack, ctx);
  if (token.kind != kind) {
    DIAGID diag = diagnostic_new(ctx->engine, token.span, ERR_UNEXPECTED_TOKEN);
    DIAGID note =
        subdiagnostic_new(diag, ctx->engine, token.span, NOTE_EXPECTED_TOKEN);
    diag_set_format(ctx->engine, note,
                    "%[file](%[row]:%[col]): %[level]: %[msg] %[0]\n");
    diag_add_arg(ctx->engine, note,
                 diagarg_sv(cstr_to_sv((char *)tok_to_str[kind])));
  }
  return token;
}
