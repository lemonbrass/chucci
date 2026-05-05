#include <da_internmap.h>
#include <lexer.h>
#include <pp2_macro.h>
#include <preprocess_2.h>
#include <token_source.h>
#include <da_string.h>
#include <thirdparty/kvec.h>
#include <setjmp.h>
#include <stdio.h>
#include <token.h>
#include <da_intern.h>
#include <compiler.h>
#include <da_path.h>

ChucciCompiler new_compiler(CompilerOpt* opt, string source, jmp_buf* onerror) {
  ChucciCompiler ctx = {0};
  imap_init(ctx.macros);
  ctx.options = opt;
  ctx.buf = new_ds();
  ctx.table = new_interntable();
  kv_push(string, ctx.source_stack, source);
  ctx.onerror = onerror;
  kv_init(ctx.included_files);
  #define X(a, b) ctx.keywords[a] = intern(ctx.table, sv_from_cstr(b));
  KEYWORDS(X)
  #undef X
  #define X(a, b) ctx.preprocessor_cmds[a] = intern(ctx.table, sv_from_cstr(b));
  PREPROCESSOR_CMD(X)
  #undef X
  return ctx;
}

void add_source(ChucciCompiler* ctx, string source) {
  kv_push(string, ctx->source_stack, source);
}

CompilerOpt* new_opt() {
  CompilerOpt* opt = malloc(sizeof(CompilerOpt));
  kv_init(opt->include_dirs);
  return opt;
}

void opt_include_dir(CompilerOpt* opt, string dir) {
  kv_push(string, opt->include_dirs, dir);
}

void free_opt(CompilerOpt** opt) {
  for (size_t i = 0; i < kv_size((*opt)->include_dirs); i++) {
    free_str(&kv_A((*opt)->include_dirs, i));
  }
  kv_destroy((*opt)->include_dirs);
  free(*opt);
  *opt = NULL;
}

TokenArray compiler_preprocess(ChucciCompiler* compiler) {
  Lexer lexer = new_lexer(compiler);
  TokenSource src = ts_from_lexer(&lexer);
  Preprocessor2 pp2 = new_pp2(compiler, &src);
  TokenArray result = resolve_pp2(&pp2);
  return result;
}

void initiate_error(ChucciCompiler* ctx) {
  longjmp(*ctx->onerror, 1);
}

void free_compiler(ChucciCompiler* ctx) {
  for (size_t i = 0; i < kv_size(ctx->included_files); i++) {
    free_path(&kv_A(ctx->included_files, i));
  }
  for (size_t i = 0; i < kv_size(ctx->source_stack); i++) {
    free_str(&kv_A(ctx->source_stack, i));
  }
  kv_destroy(ctx->included_files);
  kv_destroy(ctx->source_stack);
  imap_destroy(ctx->macros, free_macro_def);
  free_ds(&ctx->buf);
  free_interntable(&ctx->table);
  free_opt(&ctx->options);
}
