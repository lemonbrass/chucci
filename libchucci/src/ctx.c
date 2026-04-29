#include <da_path.h>
#include <da_string.h>
#include <thirdparty/kvec.h>
#include <ctx.h>


CompilerCtx new_ctx() {
  CompilerCtx ctx = {0};
  ctx.buf = new_ds();
  kv_init(ctx.include_dirs);
  kv_init(ctx.included_files);
  return ctx;
}

void free_ctx(CompilerCtx* ctx) {
  for (size_t i = 0; i < kv_size(ctx->included_files); i++) {
    free_path(&kv_A(ctx->included_files, i));
  }
  for (size_t i = 0; i < kv_size(ctx->include_dirs); i++) {
    free_str(&kv_A(ctx->include_dirs, i));
  }
  kv_destroy(ctx->include_dirs);
  kv_destroy(ctx->included_files);
  free_ds(&ctx->buf);
}
