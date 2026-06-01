#include <compiler.h>
#include <frontend/lexer.h>
#include <frontend/pp2_macro.h>
#include <frontend/preprocess_1.h>
#include <frontend/preprocess_2.h>
#include <frontend/token.h>
#include <frontend/token_source.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thirdparty/kvec.h>
#include <utils/da_arena.h>
#include <utils/da_intern.h>
#include <utils/da_internmap.h>
#include <utils/da_path.h>
#include <utils/da_string.h>

interned_str keywords[__token_kind_count] = {0};
interned_str preprocessor_cmds[__preprocessor_cmd_len] = {0};

PPCtx new_ppctx(ChucciCompiler *ctx) {
  PPCtx ppctx = {0};
  imap_init(ppctx.macros);
#define X(a, b) keywords[a] = intern(ctx->table, sv_from_cstr(b));
  KEYWORDS(X)
#undef X
#define X(a, b) preprocessor_cmds[a] = intern(ctx->table, sv_from_cstr(b));
  PREPROCESSOR_CMD(X)
#undef X

  ppctx.bufdef = malloc(sizeof(MacroDef));
  *ppctx.bufdef = (MacroDef){0};
  ppctx.global = ctx;
  return ppctx;
}

ChucciCompiler *new_compiler(CompilerOpt *opt, jmp_buf *onerror) {
  ChucciCompiler *ctx = malloc(sizeof(ChucciCompiler));
  memset(ctx, 0, sizeof(ChucciCompiler));
  ctx->options = opt;
  ctx->table = new_interntable();
  ctx->onerror = onerror;
  ctx->arena = new_arena(1024, ARENA_RELIABLE_MARK);
  ctx->ppctx = new_ppctx(ctx);
  return ctx;
}

void opt_add_source(CompilerOpt *opt, string source) {
  kv_push(string, opt->source_stack, source);
}

CompilerOpt *new_opt() {
  CompilerOpt *opt = malloc(sizeof(CompilerOpt));
  kv_init(opt->source_stack);
  kv_init(opt->include_dirs);
  return opt;
}

void opt_include_dir(CompilerOpt *opt, string dir) {
  kv_push(string, opt->include_dirs, dir);
}

void free_opt(CompilerOpt **opt) {
  for (size_t i = 0; i < kv_size((*opt)->include_dirs); i++) {
    free_str(&kv_A((*opt)->include_dirs, i));
  }
  for (size_t i = 0; i < kv_size((*opt)->source_stack); i++) {
    free_str(&kv_A((*opt)->source_stack, i));
  }
  kv_destroy((*opt)->include_dirs);
  kv_destroy((*opt)->source_stack);
  free(*opt);
  *opt = NULL;
}

TokenArray compiler_preprocess(ChucciCompiler *compiler) {
  Preprocessor1 pp1 = new_pp1(&compiler->ppctx);
  string pp1_source = resolve_pp1(&pp1);
  free_str(&kv_top(compiler->options->source_stack));
  kv_top(compiler->options->source_stack) = pp1_source;
  Lexer lexer = new_lexer(compiler);
  TokenSource src = ts_from_lexer(&lexer);
  Preprocessor2 pp2 = new_pp2(compiler, &src);
  TokenArray result = resolve_pp2(&pp2);
  return result;
}

void initiate_error(ChucciCompiler *ctx) { longjmp(*ctx->onerror, 1); }

void free_ppctx(PPCtx *ppctx) {
  for (size_t i = 0; i < kv_size(ppctx->included_files); i++) {
    free_path(&kv_A(ppctx->included_files, i));
  }
  kv_destroy(ppctx->included_files);
  imap_destroy(ppctx->macros, free_macro_def);
  kv_destroy(ppctx->macro_stack);
  *ppctx = (PPCtx){0};
}

void free_compiler(ChucciCompiler **ctx) {
  arena_free((*ctx)->arena);
  free_interntable(&(*ctx)->table);
  free_opt(&(*ctx)->options);
  free_ppctx(&(*ctx)->ppctx);
  free(*ctx);
  *ctx = NULL;
}
