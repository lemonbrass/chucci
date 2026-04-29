#include "thirdparty/kvec.h"
#include <ctx.h>


CompilerCtx new_ctx() {
  CompilerCtx ctx = {0};
  kv_init(ctx.include_dirs);
  kv_init(ctx.included_files);
  return ctx;
}
