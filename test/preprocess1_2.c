#include <compiler.h>
#include <ctx.h>
#include <da_string.h>
#include <preprocess_1.h>
#include <setjmp.h>

extern jmp_buf assert_env;

/*
 NOTE: THIS CODE CURRENTLY LEAKS MEMORY BECAUSE OF assert() INSTEAD OF PROPER ERROR HANDLING
*/

void preprocess1_2(jmp_buf errbuf) {
  const char* source = "#include <../test/inputs/recursiveinclude.h>";
  CompilerOpt* opt = new_opt();
  opt_include_dir(opt, str_from_cstr_copy("."));
  CompilerCtx ctx = new_ctx(opt);
  Preprocessor1 pp1 = new_pp1(sv_from_cstr(source), &ctx);
  
  if (setjmp(assert_env) == 0) {
    string result = resolve_pp1(&pp1);
    free_ctx(&ctx);
    free_opt(&opt);
    free_str(&result);
    // We expect the code to throw an error
    // If it doesnt, its a failure
    longjmp(errbuf, 1);
  }
  else {
    // test passed
    free_ctx(&ctx);
    free_opt(&opt);
  }
}
