#include <token.h>
#include <compiler.h>
#include <da_string.h>
#include <lexer.h>
#include <setjmp.h>


void lexer1(jmp_buf errbuf) {
  jmp_buf onerror;
  string source = str_from_cstr_copy(
    "int x = 0.6.9 + 42.0;\n"
    "string y = \"Hello, World\";\n"
  );
  CompilerOpt* opt = new_opt();
  ChucciCompiler compiler = new_compiler(opt, source, &onerror);

  TokenKind expected[] = { KW_INT, TOK_IDENT, OP_ASSIGN, TOK_VAL, OP_DOT, TOK_VAL, OP_ADD, TOK_VAL, SEP_SEMI, SEP_NEWLINE,
                           TOK_IDENT, TOK_IDENT, OP_ASSIGN, TOK_VAL, SEP_SEMI, SEP_NEWLINE, TOK_EOF };

  if (setjmp(onerror) == 0) {
    Lexer lexer = new_lexer(&compiler);

    size_t i = 0;
    for (Token token = lex_next_token(&lexer); ; token = lex_next_token(&lexer)) {
      // print_token_pretty(&token);
      // printf("\n");
      if (token.kind != expected[i]) {
        free_compiler(&compiler);
        longjmp(errbuf, 1);
      }
      if (token.kind == TOK_EOF) break;
      i++;
    }
  }
  else {
    free_compiler(&compiler);
    longjmp(errbuf, 1);
  }
  free_compiler(&compiler);
}
