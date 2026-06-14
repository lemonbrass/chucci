#include "frontend/preprocessor.h"
#include "compiler.h"
#include "frontend/pp_macro.h"
#include "frontend/token.h"
#include "frontend/token_stream.h"
#include "utils/diagnostics.h"
#include "utils/vmem_arena.h"
#include <stdio.h>

StringID ppcmd_to_id[__preprocessor_cmd_len];

#define PP_VMARENA_CAP 1024 * 1024

Preprocessor *pp_new(CompilerCtx *ctx, Lexer *lexer) {
#define X(kind, str)                                                           \
  ppcmd_to_id[kind] = intern(const_cstr_to_sv(str), ctx->interner);
  PREPROCESSOR_CMD(X)
#undef X
  Preprocessor *pp = vmarena_calloc(ctx->arena, sizeof(Preprocessor));
  pp->streams = ts_stack_new();
  pp->macros = macrodefmap_new();
  pp->arena = vmarena_new(PP_VMARENA_CAP);
  TokenStream ts = ts_from_lexer(lexer);
  ts_stack_push(&pp->streams, ts, ctx);
  return pp;
}

void pp_free(Preprocessor *pp) {
  printf("Preprocessor Arena allocated: %zu/%zu\n", pp->arena->pos,
         pp->arena->cap);
  vmarena_free(pp->arena);
}

void preprocess(Preprocessor *pp, CompilerCtx *ctx) {
  Token token = ts_stack_next_token(&pp->streams, ctx);
  switch (token.kind) {
  case TOK_IDENT:
    if (ppcmd_to_id[PP_DEFINE] == token.ident) {
      macro_def(pp, ctx, token);
      break;
    }
  default:
    (void)0;
    diagnostic_new(ctx->engine, token.span,
                   ERR_UNEXPECTED_TOKEN_AFTER_OP_PREPROCESS);
  }
}

Token pp_next_token(Preprocessor *pp, CompilerCtx *ctx) {
  Token token = ts_stack_next_token(&pp->streams, ctx);
  if (token.kind == OP_PREPROCESS) {
    preprocess(pp, ctx);
    return pp_next_token(pp, ctx);
  }
  // Macro use
  else if (token.kind == TOK_IDENT) {
    MacroDef *def = macrodefmap_get(&pp->macros, token.ident);
    if (def) {
      TokenStream ts = macro_use(pp, ctx, token, def);
      ts_stack_push(&pp->streams, ts, ctx->arena);
      return pp_next_token(pp, ctx);
    } else
      return token;
  }
  // skip newlines, they are only relevant for preprocessor
  else if (token.kind == SEP_NEWLINE) {
    return pp_next_token(pp, ctx);
  }
  // else, just return the token
  else
    return token;
}
