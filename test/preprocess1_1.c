#include "ctx.h"
#include "da_path.h"
#include "thirdparty/kvec.h"
#include <da_string.h>
#include <preprocess_1.h>
#include <da_arena.h>
#include <setjmp.h>

void preprocess1_1(jmp_buf errbuf) {
  string_view source = sv_from_cstr(
      "#include <test/inputs/yoyo.h>\n"
      "//This comment wont make it out alive :D \\\n"
      " This comment continues longer than you think....\n"
      "int main() {\n"
      "   return yoyo_ret();\n"
      "}\n"
  );
  CompilerCtx ctx = new_ctx();
  Path cwdpath = new_path(sv_from_cstr("../.")); // move out of build folder
  string_view cwd = get_absolute_path(&cwdpath);
  kv_push(string_view, ctx.include_dirs, cwd);
  
  Preprocessor1 pp1 = new_pp1(source, &ctx);
  
  string_view result = resolve_pp1(&pp1);
  
  printf("Preprocessor1 result: \n%.*s\n", (int)result.len, result.str);
  
  free_path(&cwdpath);
  free_sv(&cwd);
}

