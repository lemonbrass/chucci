#include <thirdparty/kvec.h>
#include <token.h>
#include <token_source.h>
#include <compiler.h>
#include <da_string.h>
#include <setjmp.h>
#include <stdio.h>

int main() {
  jmp_buf onerror;
  CompilerOpt* opt = new_opt();
  opt_include_dir(opt, str_from_cstr_copy("."));
  ChucciCompiler ctx = new_compiler(opt, str_from_cstr_copy(""), &onerror);
  da_string dsinput = new_ds();
  if (setjmp(onerror) == 0) {
    while (true) {
      printf("chuccic> ");
      int ch;
      while ((ch = getchar()) != EOF && ch != '\n') {
        push_char_ds(&dsinput, (unsigned char)ch);
      }
      if (s_eq(ds_to_sv(&dsinput), sv_from_cstr("exit"))) break;
      push_char_ds(&dsinput, '\0');
      string source = ds_to_str_copy(&dsinput);
      add_source(&ctx, source);
      if (s_eq(source, sv_from_cstr("exit"))) break;

      TokenArray result = compiler_compile(&ctx);
      print_token_array(&result);
      kv_destroy(result);
      reset_ds(&dsinput);
    }
    free_ds(&dsinput);
    free_compiler(&ctx);
  }
  else {
    free_ds(&dsinput);
    free_compiler(&ctx);
    return 1;
  }
  return 0;
}
