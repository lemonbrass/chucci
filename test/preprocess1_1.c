#include "da_path.h"
#include "thirdparty/kvec.h"
#include <da_string.h>
#include <preprocess_1.h>
#include <da_arena.h>
#include <setjmp.h>

void preprocess1_1(jmp_buf errbuf) {
  string_view source = sv_from_cstr(
      "#include <test/inputs/yoyo.h>\n"
      "int main() {\n"
      "   return yoyo_ret();\n"
      "}\n"
  );
  Preprocessor1 pp1 = new_pp1(source);
  
  Path cwdpath = new_path(sv_from_cstr("../.")); // move out of build folder
  string_view cwd = get_absolute_path(&cwdpath);
  
  kv_push(string_view, pp1.include_dirs, cwd);
  string_view result = resolve_pp1(&pp1);
  
  printf("Preprocessor1 result: %.*s\n", (int)result.len, result.str);
  
  free_path(&cwdpath);
  free_sv(&cwd);
}

