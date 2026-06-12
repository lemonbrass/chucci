#include "frontend/pp_macro.h"
#include "frontend/preprocessor.h"
#include "frontend/token.h"
#include "frontend/token_stream.h"
#include "utils/linear_map.h"

LINEAR_MAP_IMPL(StringID, MacroDef, MacroDefMap, macrodefmap,
                VMEM_ARENA_ALLOC_INT)
#define stack(pp) &(pp)->streams

void macro_def(Preprocessor *pp, CompilerCtx *ctx, Token token) {
  MacroDef def = {0};
  Token name = ts_stack_expect_token(stack(pp), ctx, TOK_IDENT);
  Token next = ts_stack_peek_token(stack(pp), ctx);
  if (next.kind == SEP_LPAREN && name.span.start + 1 == next.span.start) {
    // Function Like Macro
    def.is_fnlike = true;
    return;
  }
  def.body = tokenvec_new();
  while (next.kind != SEP_NEWLINE && next.kind != TOK_EOF) {
    next = ts_stack_next_token(stack(pp), ctx);
    tokenvec_push(&def.body, next, ctx->arena);
  }
  macrodefmap_set(&pp->macros, name.ident, def, ctx->arena);
}
