#include <token.h>
#include <compiler.h>
#include <ctx.h>
#include <da_string.h>
#include <lexer.h>
#include <setjmp.h>


void lexer1(jmp_buf errbuf) {
  string_view source = sv_from_cstr(
    "int x = 0.6.9 + 42.0;"
  );
  CompilerOpt* opt = new_opt();
  CompilerCtx ctx = new_ctx(opt);

  TokenKind expected[] = { KW_INT, TOK_IDENT, OP_ASSIGN, TOK_NUM, OP_DOT, TOK_NUM, OP_ADD, TOK_NUM, SEP_SEMI, TOK_EOF };
  Lexer lexer = new_lexer(source, &ctx);

  size_t i = 0;
  for (Token token = lex_next_token(&lexer); ; token = lex_next_token(&lexer)) {
    // print_token(&token);
    // printf("\n");
    if (token.kind != expected[i]) {
      free_ctx(&ctx);
      free_opt(&opt);
      longjmp(errbuf, 1);
    }
    if (token.kind == TOK_EOF) break;
    i++;
  }

  free_ctx(&ctx);
  free_opt(&opt);
}
