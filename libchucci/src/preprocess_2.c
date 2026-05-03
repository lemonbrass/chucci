#include "da_internmap.h"
#include <da_intern.h>
#include <lexer.h>
#include <token.h>
#include <thirdparty/kvec.h>
#include <preprocess_2.h>



Preprocessor2 new_pp2(CompilerCtx* ctx) {
  Preprocessor2 pp2 = {0};
  pp2.ctx = ctx;
  return pp2;
}

void step_pp2(Preprocessor2* pp2, Lexer* lexer, Token* tok) {
  switch (tok->kind) {
    case OP_PREPROCESS:
      *tok = expect_token_kind(lexer, TOK_IDENT);
      if (interned_eq(tok->ident, pp2->ctx->preprocessor_cmds[PP_DEFINE])) {
        macro_def(pp2, lexer);
        break;
      } else {
        assert(false && "UNIMPLEMENTED");
      }
      break;
    case TOK_IDENT:
      // TODO: imap_get change
      imap_get(pp2->macros, tok->ident);
    default:
      kv_push(Token, pp2->stream, *tok);
  }
  *tok = lex_next_token(lexer);
}

TokenStream resolve_pp2(Preprocessor2* pp2, Lexer* lexer) {
  Token tok = lex_next_token(lexer);
  while (tok.kind != TOK_EOF) {
    step_pp2(pp2, lexer, &tok);
  }
  kv_push(Token, pp2->stream, tok);
  return pp2->stream;
}
