#include "lexer.h"
#include "thirdparty/kvec.h"
#include "token.h"
#include "token_source.h"
#include <preprocess_2.h>
#include <ctx.h>
#include <compiler.h>
#include <da_string.h>
#include <setjmp.h>
#include <stdio.h>


void preprocess2_1(jmp_buf errbuf) {
  jmp_buf onerror;
  string source = str_from_cstr_copy(
    "#define MY_CONST1 69\n"
    "#define MY_CONST2 420\n"
    "#define ADD(x, y) x + y\n"
    "printf(\"%d + %d = %d \\n\", MY_CONST1, MY_CONST2, ADD(MY_CONST1, MY_CONST2));\n"
  );
  assert(source.cstr);
  CompilerOpt* opt = new_opt();
  CompilerCtx ctx = new_ctx(opt, source, &onerror);

  if (setjmp(onerror) == 0) {
    Lexer lexer = new_lexer(&ctx);
    TokenSource src = ts_from_lexer(&lexer);
    Preprocessor2 pp2 = new_pp2(&ctx, &src);

    TokenArray result = resolve_pp2(&pp2);

    for (size_t i=0; i<kv_size(result); i++) {
      Token token = kv_A(result, i);
      print_token_pretty(&token);
      if (token.kind == SEP_NEWLINE) printf("\n");
      else printf(" ");
    }

    kv_destroy(result);
    free_pp2(&pp2);
    free_ctx(&ctx);
    free_opt(&opt);
  }
  else {
    free_ctx(&ctx);
    free_opt(&opt);
    longjmp(errbuf, 1);
  }
}
