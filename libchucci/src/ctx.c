#include <token.h>
#include <da_intern.h>
#include <compiler.h>
#include <da_path.h>
#include <da_string.h>
#include <thirdparty/kvec.h>
#include <ctx.h>


CompilerCtx new_ctx(CompilerOpt* options) {
  CompilerCtx ctx = {0};
  ctx.options = options;
  ctx.buf = new_ds();
  ctx.table = new_interntable();
  kv_init(ctx.included_files);
  #define X(a, b) ctx.keywords[a] = intern(ctx.table, sv_from_cstr(b));
  KEYWORDS(X)
  #undef X
  #define X(a, b) ctx.preprocessor_cmds[a] = intern(ctx.table, sv_from_cstr(b));
  PREPROCESSOR_CMD(X)
  #undef X
  return ctx;
}

void free_ctx(CompilerCtx* ctx) {
  for (size_t i = 0; i < kv_size(ctx->included_files); i++) {
    free_path(&kv_A(ctx->included_files, i));
  }
  kv_destroy(ctx->included_files);
  free_ds(&ctx->buf);
  free_interntable(&ctx->table);
}
