#include <compiler.h>
#include <ctx.h>
#include <thirdparty/kvec.h>
#include <da_string.h>
#include <preprocess_1.h>
#include <da_arena.h>
#include <setjmp.h>


void preprocess1_1(jmp_buf errbuf) {
  jmp_buf onerror;
  string_view expected = sv_from_cstr(
    "int yoyo_ret() {\n"
    "  return 69420;\n"
    "}\n"
    "\n"
    "\n"
    "int main() {\n"
    "   return yoyo_ret();\n"
    "}\n"
  );
  string source = str_from_cstr_copy(
      "#include <../test/inputs/yoyo.h>\n"
      "//This comment wont make it out alive :D \\\n"
      " This comment continues longer than you think....\n"
      "int main() {\n"
      "   return yoyo_ret();\n"
      "}\n"
  );

  CompilerOpt* opt = new_opt();
  opt_include_dir(opt, str_from_cstr_copy("."));
  CompilerCtx ctx = new_ctx(opt, source, &onerror);

  if (setjmp(onerror) == 0) {
    Preprocessor1 pp1 = new_pp1(&ctx);
    string result = resolve_pp1(&pp1);
  
    // printf("Preprocessor1 result: \n[%.*s]\nexpected: \n[%.*s]\n", (int)result.len, result.cstr, (int)expected.len, expected.cstr);

    free_ctx(&ctx);
    free_opt(&opt);
    if (!s_eq(expected, result)) {
      free_str(&result);
      printf("Error: Preprocessor1 result doesnt match expected result.\n");
      longjmp(errbuf, 1);
    }
    free_str(&result);
  }
  else {
    free_ctx(&ctx);
    free_opt(&opt);
    longjmp(errbuf, 1);
  }
}

