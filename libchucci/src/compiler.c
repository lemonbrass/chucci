#include "compiler.h"
#include "frontend/lexer.h"
#include "frontend/preprocessor.h"
#include "frontend/token.h"
#include "utils/chucci_alloc.h"
#include "utils/diagnostics.h"
#include "utils/file.h"
#include "utils/smallvec.h"
#include "utils/string.h"
#include "utils/string_interner.h"
#include "utils/vmem_arena.h"
#include <setjmp.h>

SMALLVEC_IMPL(String, StringVec, stringvec, VMEM_ARENA_ALLOC_INT)

CompilerCtx *compiler_new(jmp_buf *onerror) {
  VMEMArena *arena = vmarena_new(VMEM_ARENA_MAX_CAP);
  CompilerCtx *ctx = vmarena_calloc(arena, sizeof(CompilerCtx));
  ctx->arena = arena;
  ctx->engine = diagnostic_engine_new(arena);
  ctx->interner = interner_new(arena);
  ctx->onerror = onerror;
  return ctx;
}

void cc_add_source(CompilerCtx *ctx, File source) {
  filevec_push(&ctx->sources, source, ctx->arena);
}

void cc_preamble(CompilerCtx *ctx) {
  assert(ctx->sources.len > 0);
  ctx->lexer = lexer_new(ctx);
  ctx->preprocessor = pp_new(ctx, ctx->lexer);
}

void cc_compile(CompilerCtx *ctx) {
  assert(ctx->sources.len > 0);
  // Testing code for now, till compiler is complete
  Token token = {0};
  while (token.kind != TOK_EOF) {
    token = pp_next_token(ctx->preprocessor, ctx);
    print_token_pretty(&token);
    printf(" ");
    if (has_fatal_diagnostics(ctx->engine)) {
      printf("\n");
      diagnostics_emit(ctx->engine);
      longjmp(*ctx->onerror, 1);
    }
  }
  printf("\n");
  if (has_diagnostics(ctx->engine)) {
    diagnostics_emit(ctx->engine);
    longjmp(*ctx->onerror, 1);
  }
}

void cc_free(CompilerCtx *ctx) {
  pp_free(ctx->preprocessor);
  interner_free(ctx->interner);
  diagnostic_engine_free(ctx->engine);
  filevec_free(&ctx->sources, &ctx->arena);
  stringvec_free(&ctx->included_dirs, &ctx->arena);
  vmarena_free(ctx->arena);
}
