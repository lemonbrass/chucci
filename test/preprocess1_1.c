#include <da_string.h>
#include <preprocess_1.h>
#include <da_arena.h>
#include <setjmp.h>

void preprocess1_1(jmp_buf errbuf) {
  string_view source = sv_from_cstr(
      "#include <yoyo.h>\n"
      "int main() {\n"
      "   return yoyo_errcode();\n"
      "}\n"
  );
  arena_t* arena = new_arena(1024*16, ARENA_RELIABLE_MARK);
  Preprocessor1 pp1 = new_pp1(source, arena);
  string_view result = resolve_pp1(&pp1);
  printf("Preprocessor1 result: %.*s\n", (int)result.len, result.str);
}

