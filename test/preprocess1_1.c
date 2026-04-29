#include <ctx.h>
#include <thirdparty/kvec.h>
#include <da_string.h>
#include <preprocess_1.h>
#include <da_arena.h>
#include <setjmp.h>

void preprocess1_1(jmp_buf errbuf) {
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
  string_view source = sv_from_cstr(
      "#include <../test/inputs/yoyo.h>\n"
      "//This comment wont make it out alive :D \\\n"
      " This comment continues longer than you think....\n"
      "int main() {\n"
      "   return yoyo_ret();\n"
      "}\n"
  );
  CompilerCtx ctx = new_ctx();
  kv_push(string_view, ctx.include_dirs, sv_from_cstr(strdup("."))); // CTX FREES string_view LATER, SO WE NEED MALLOCED MEMORY
  
  Preprocessor1 pp1 = new_pp1(source, &ctx);
  
  string_view result = resolve_pp1(&pp1);
  
  printf("Preprocessor1 result: \n[%.*s]\n expected: \n[%.*s]\n", (int)result.len, result.str, (int)expected.len, expected.str);

  if (s_cmp(expected, result)) longjmp(errbuf, 1);
  
  free_ctx(&ctx);
}

