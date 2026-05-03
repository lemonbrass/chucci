#include <lexer.h>
#include <ctx.h>
#include <compiler.h>
#include <da_string.h>
#include <setjmp.h>


void lexer2(jmp_buf errbuf) {
  string_view source = sv_from_cstr(
    ""
  );
  CompilerOpt* opt = new_opt();
  CompilerCtx ctx = new_ctx(opt);
  Lexer lexer = new_lexer(source, &ctx);
  
}
