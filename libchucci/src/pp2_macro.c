#include <da_internmap.h>
#include <da_intern.h>
#include <thirdparty/kvec.h>
#include <token.h>
#include <pp2_macro.h>
#include <preprocess_2.h>
#include <lexer.h>


void macro_def(Preprocessor2* pp2, Lexer* lexer) {
  MacroDef def = {0};
  def.name = expect_token_kind(lexer, TOK_IDENT).ident;
  Token token = expect_token_kind(lexer, SEP_LPAREN);
  while (true) {
    token = expect_token_kind(lexer, TOK_IDENT);
    kv_push(interned_str, def.args, token.ident);

    token = lex_next_token(lexer);
    if (token.kind == SEP_COMMA) continue;
    else if (token.kind == SEP_RPAREN) break;
    else throw_lexer_error(lexer, token, "Unexpected token in macro definition");
  }

  while (true) {
    token = lex_next_token(lexer);
    if (token.kind == SEP_NEWLINE || token.kind == TOK_EOF) break;
    kv_push(Token, def.body, token);
  }
  imap_set(def, pp2->macros, def.name);
}
