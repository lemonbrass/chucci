#include "compiler.h"
#include "pp2_cond.h"
#include <da_intern.h>
#include <da_internmap.h>
#include <da_string.h>
#include <lexer.h>
#include <pp2_macro.h>
#include <preprocess_2.h>
#include <thirdparty/kvec.h>
#include <token.h>
#include <token_source.h>

#define globalctx pp2->ctx->global

Preprocessor2 new_pp2(PPCtx *ctx, TokenSource *token_source) {
  Preprocessor2 pp2 = {0};
  pp2.ctx = ctx;
  pp2.token_source = token_source;
  return pp2;
}

void step_pp2(Preprocessor2 *pp2, Token *tok) {
  switch (tok->kind) {
  case OP_PREPROCESS:
    *tok = next_token(pp2->token_source);
    if (interned_eq(tok->ident, preprocessor_cmds[PP_DEFINE])) {
      macro_def(pp2);
      break;
    } else if (tok->kind == KW_IF) {
      cond_use(pp2);
      break;
    } else {
      throw_error(pp2->token_source, *tok, "Unexpected token", globalctx);
    }
    break;
  case TOK_IDENT:
    if (imap_has(pp2->ctx->macros, tok->ident) == 1) {
      macro_use(pp2, tok, &pp2->stream);
      break;
    }
  default:
    if (tok->kind != SEP_NEWLINE)
      kv_push(Token, pp2->stream, *tok);
  }
  *tok = next_token(pp2->token_source);
}

TokenArray resolve_pp2(Preprocessor2 *pp2) {
  Token tok = next_token(pp2->token_source);
  while (tok.kind != TOK_EOF) {
    step_pp2(pp2, &tok);
  }
  return pp2->stream;
}

TokenArray recursively_expand(Preprocessor2 *pp2, TokenSource *token_source) {
  Preprocessor2 child = {0};
  child.token_source = token_source;
  child.ctx = pp2->ctx;
  return resolve_pp2(&child);
}
