#include "frontend/pp_macro.h"
#include "compiler.h"
#include "frontend/preprocessor.h"
#include "frontend/token.h"
#include "frontend/token_stream.h"
#include "utils/diagnostics.h"
#include "utils/linear_map.h"
#include "utils/string.h"
#include "utils/string_interner.h"
#include <stdio.h>
#include <string.h>

LINEAR_MAP_IMPL(StringID, MacroDef, MacroDefMap, macrodefmap,
                VMEM_ARENA_ALLOC_INT)
#define stack(pp) &(pp)->streams

void macrodef_free(MacroDef *def, CompilerCtx *ctx) {
  stridvec_free(&def->args, ctx->arena);
  tokenvec_free(&def->body, ctx->arena);
}

void print_macro_def(Preprocessor *pp, CompilerCtx *ctx, MacroDef *def) {
  printf("DEF{\n");
  StringID *name = macrodefmap_get_key(&pp->macros, *def);
  assert(name);
  StringView name_sv = get_interned_sv(ctx->interner, *name);
  anystr_print(name_sv);
  if (def->is_fnlike) {
    putchar('(');
    for (size_t i = 0; i < def->args.len; i++) {
      StringView sv =
          get_interned_sv(ctx->interner, stridvec_access(&def->args, i));
      anystr_print(sv);
      if (i < def->args.len - 1)
        printf(", ");
    }
    putchar(')');
  }
  printf(": ");
  for (size_t i = 0; i < def->body.len; i++) {
    print_token_pretty(tokenvec_access_ptr(&def->body, i));
    putchar(' ');
  }
  putchar('\n');
  printf("}\n");
}

void macro_def_args(Preprocessor *pp, CompilerCtx *ctx, MacroDef *def,
                    bool *has_error) {
  Token next = ts_stack_next_token(stack(pp), ctx);
  bool expect_comma = false;
  while (next.kind != SEP_RPAREN && next.kind != TOK_EOF) {
    if (next.kind == TOK_IDENT) {
      stridvec_push(&def->args, next.ident, ctx->arena);
      expect_comma = true;
    } else if (next.kind == SEP_COMMA && expect_comma) {
      expect_comma = false;
    } else {
      *has_error = true;
      diagnostic_new(ctx->engine, next.span, ERR_INVALID_MACRO_DEF);
    }
    next = ts_stack_next_token(stack(pp), ctx);
  }
  if (next.kind != SEP_RPAREN) {
    *has_error = true;
    diagnostic_new(ctx->engine, next.span, ERR_INVALID_MACRO_DEF);
  }
}

void macro_def(Preprocessor *pp, CompilerCtx *ctx, Token token) {
  MacroDef def = {0};
  Token name = ts_stack_expect_token(stack(pp), ctx, TOK_IDENT);
  Token next = ts_stack_peek_token(stack(pp), ctx);
  bool has_error = name.kind != TOK_IDENT;
  // Function Like Macro
  if (next.kind == SEP_LPAREN &&
      name.span.start + name.span.len == next.span.start) {

    def.is_fnlike = true;
    next = ts_stack_next_token(stack(pp), ctx); // skip (

    macro_def_args(pp, ctx, &def, &has_error);
  }
  def.body = tokenvec_new();
  while (next.kind != SEP_NEWLINE && next.kind != TOK_EOF) {
    next = ts_stack_next_token(stack(pp), ctx);
    if (!has_error)
      tokenvec_push(&def.body, next, ctx->arena);
  }
  if (has_error) {
    macrodef_free(&def, ctx);
    return;
  }
  macrodefmap_set(&pp->macros, name.ident, def, ctx->arena);
  print_macro_def(pp, ctx, &def);
}
