#include <da_string.h>
#include <pp2_macro.h>
#include <da_internmap.h>
#include <da_intern.h>
#include <lexer.h>
#include <token.h>
#include <thirdparty/kvec.h>
#include <preprocess_2.h>
#include <token_source.h>


Preprocessor2 new_pp2(CompilerCtx* ctx, TokenSource* token_source) {
  Preprocessor2 pp2 = {0};
  pp2.ctx = ctx;
  pp2.token_source = token_source;
  return pp2;
}

void step_pp2(Preprocessor2* pp2, Token* tok) {
  switch (tok->kind) {
    case OP_PREPROCESS:
      *tok = expect_token_kind(pp2->token_source, TOK_IDENT);
      if (interned_eq(tok->ident, pp2->ctx->preprocessor_cmds[PP_DEFINE])) {
        macro_def(pp2);
        break;
      } else {
        assert(false && "UNIMPLEMENTED");
      }
      break;
    case TOK_IDENT:
      if (imap_has(pp2->macros, tok->ident)) {
        macro_use(pp2, tok);
        break;
      }
    default:
      kv_push(Token, pp2->stream, *tok);
  }
  *tok = next_token(pp2->token_source);
}

TokenArray resolve_pp2(Preprocessor2* pp2) {
  Token tok = next_token(pp2->token_source);
  while (tok.kind != TOK_EOF) {
    step_pp2(pp2, &tok);
  }
  kv_push(Token, pp2->stream, tok);
  return pp2->stream;
}
