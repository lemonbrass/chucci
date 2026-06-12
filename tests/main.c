#include "compiler.h"
#include "utils/string.h"
#include <setjmp.h>
#include <stdio.h>

jmp_buf onerror;

int main() {
  CompilerCtx *ctx = compiler_new(&onerror);
  char *source = "int // \n x_y_haha = \"HUiHUi\" /* ohhh yeahhh */ #define "
                 "69 01\n; \\\n int x "
                 "= 1.1.1 / 1; 1.2.34.4;";
  File file = {.contents = cstr_to_str(source),
               .name = const_cstr_to_sv("scratch")};

  cc_add_source(ctx, file);

  if (setjmp(onerror) == 0) {
    cc_preamble(ctx);
    cc_compile(ctx);
  } else {
    // error
  }
  printf("Arena allocated: %zu/%zu\n", ctx->arena->pos, ctx->arena->cap);
  cc_free(ctx);
}
