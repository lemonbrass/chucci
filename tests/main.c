#include "compiler.h"
#include "utils/string.h"
#include <setjmp.h>

jmp_buf onerror;

int main() {
  CompilerCtx *ctx = compiler_new(&onerror);
  char *source = "int // \n x_y_haha = \"HUiHUi\" /* ohhh yeahhh */; \n int x "
                 "= 1.1.1.1 / 1; 1.2.34.4;";
  File file = {.contents = cstr_to_anystr(source, String),
               .name = cstr_to_anystr("scratch", StringView)};

  cc_add_source(ctx, file);

  if (setjmp(onerror) == 0) {
    cc_preamble(ctx);
    cc_compile(ctx);
  } else {
    // error
  }
  cc_free(ctx);
}
