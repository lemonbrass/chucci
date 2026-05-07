#include <lexer.h>
#include <thirdparty/kvec.h>
#include <token.h>
#include <token_source.h>
#include <preprocess_2.h>
#include <compiler.h>
#include <da_string.h>
#include <setjmp.h>
#include <stdio.h>


void preprocess2_1(jmp_buf errbuf) {
  jmp_buf onerror;
  string source = str_from_cstr_copy(
    "#define MY_CONST1 69\n"
    "#define MY_CONST2 420\n"
    "#define ADD(x, y) x + y\n"
    "printf(\"%d + %d = %d \\n\", MY_CONST1, MY_CONST2, ADD(MY_CONST1, MY_CONST2));\n"
  );
  assert(source.cstr);
  CompilerOpt* opt = new_opt();
  ChucciCompiler compiler = new_compiler(opt, source, &onerror);

  if (setjmp(onerror) == 0) {
    TokenArray result = compiler_compile(&compiler);
    
    print_token_array(&result);

    kv_destroy(result);
    free_compiler(&compiler);
  }
  else {
    longjmp(errbuf, 1);
    free_compiler(&compiler);
  }
}
