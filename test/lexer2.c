#include <lexer.h>
#include <ctx.h>
#include <compiler.h>
#include <da_string.h>
#include <setjmp.h>


void lexer2(jmp_buf errbuf) {
  jmp_buf onerror;
  string source = str_from_cstr_copy(
    ""
  );
  CompilerOpt* opt = new_opt();
  CompilerCtx ctx = new_ctx(opt, source, &onerror);
  if (setjmp(onerror) == 0) {
    Lexer lexer = new_lexer(&ctx);
    free_ctx(&ctx);
    free_opt(&opt);
  }
  else {
    longjmp(errbuf, 1);
    free_ctx(&ctx);
    free_opt(&opt);
  }
}
