#include "frontend/preprocessor.h"
#include "compiler.h"
#include "frontend/token.h"
#include "frontend/token_stream.h"
#include "utils/diagnostics.h"
#include "utils/vmem_arena.h"

Preprocessor *pp_new(CompilerCtx *ctx, Lexer *lexer) {
  Preprocessor *pp = vmarena_calloc(ctx->arena, sizeof(Preprocessor));
  pp->streams = ts_stack_new();
  TokenStream ts = ts_from_lexer(lexer);
  ts_stack_push(&pp->streams, ts, ctx);
  return pp;
}

Token preprocess(Preprocessor *pp, CompilerCtx *ctx) {
  Token token = ts_stack_next_token(&pp->streams, ctx);
  switch (token.kind) {
  default:
    (void)0;
    Diagnostic diag =
        diagnostic_new(ERR_UNEXPECTED_TOKEN_AFTER_OP_PREPROCESS,
                       cursor_from_token(token), token.pos, token.pos);
    diagnostic_add(ctx->engine, diag);
    (void)0;
  }
}

Token pp_next_token(Preprocessor *pp, CompilerCtx *ctx) {
  Token token = ts_stack_next_token(&pp->streams, ctx);
  if (token.kind == OP_PREPROCESS)
    return preprocess(pp, ctx);
  else
    return token;
}
