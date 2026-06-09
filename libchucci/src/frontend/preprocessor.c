#include "frontend/preprocessor.h"
#include "compiler.h"
#include "frontend/token_stream.h"
#include "utils/vmem_arena.h"

Preprocessor *pp_new(CompilerCtx *ctx, Lexer *lexer) {
  Preprocessor *pp = vmarena_calloc(ctx->arena, sizeof(Preprocessor));
  pp->streams = ts_stack_new();
  TokenStream ts = ts_from_lexer(lexer);
  ts_stack_push(&pp->streams, ts, ctx);
  return pp;
}

Token pp_next_token(Preprocessor *pp, CompilerCtx *ctx) {
  Token token = ts_stack_next_token(&pp->streams, ctx);
  return token;
}
