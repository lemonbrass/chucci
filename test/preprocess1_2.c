#include <compiler.h>
#include <da_string.h>
#include <preprocess_1.h>
#include <setjmp.h>


void preprocess1_2(jmp_buf errbuf) {
  jmp_buf onerror;
  const char* source = "#include <../test/inputs/recursiveinclude.h>";
  CompilerOpt* opt = new_opt();
  opt_include_dir(opt, str_from_cstr_copy("."));
  ChucciCompiler ctx = new_compiler(opt, str_from_cstr_copy(source), &onerror);
  if (setjmp(onerror) == 0) {
    Preprocessor1 pp1 = new_pp1(&ctx);
  
    string result = resolve_pp1(&pp1);
    free_compiler(&ctx);
    free_str(&result);
    // We expect the code to throw an error
    // If it doesnt, its a failure
    longjmp(errbuf, 1);
  }
  else {
    // test passed
    free_compiler(&ctx);
  }
}
