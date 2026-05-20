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
    "#define add(x,y) (x + y)\n"
    "#define str1(a) #a\n"
    "#define str2(a) str1(a)\n"
    "#define A(a, b, c) a##b = str2(c);\n"
    "#define B(a, b, c) A(a, b, c) A(c, b, a)\n"
    "#define X Y\n"
    "#define Y X\n"
    "X\n"
    "Y\n"
    "A(x, y, z)\n"
    "B(x, y, z)\n"
    "add(add(1, 2), 3)\n"
    "str2(add(1, 2) * 1*2/7)\n"
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
