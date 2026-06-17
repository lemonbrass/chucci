#include <setjmp.h>
#include <stdio.h>

#include "compiler.h"
#include "utils/string.h"

jmp_buf onerror;

int main() {
  CompilerCtx* ctx = compiler_new(&onerror);
  char* source =
      "int z = 1.1.2.3.4;\n"
      "int x_y = z;\n"
      "/*this is ignored HHAHHAHAHAHH I CAN SAY WHATEVER AND YOU WONT HEAR*/\n"
      "#define x 69\n"
      "#define y(a, b) ((a) + (b))\n"
      "#define stick(a, b) a##b\n"
      "#define stringify(a) #a\n"
      "stick(haha, huhu);\n"
      "stringify(ahahhahaa);\n"
      "print(x + y(1 + 2, 2));\n"
      "#define 69 x\n"
      "#define 67 y\n";
  File file = {.contents = cstr_to_sv(source),
               .name = const_cstr_to_sv("scratch")};

  cc_add_source(ctx, file);

  if (setjmp(onerror) == 0) {
    printf("Compiling input: \n{\n%s\n}\n", source);
    cc_preamble(ctx);
    cc_compile(ctx);
  } else {
    // error
  }
  printf("Arena allocated: %zu/%zu\n", ctx->arena->pos, ctx->arena->cap);
  cc_free(ctx);
}
