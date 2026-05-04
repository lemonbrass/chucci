#include "da_string.h"
#include <token_source.h>
#include <assert.h>
#include <da_internmap.h>
#include <da_intern.h>
#include <thirdparty/kvec.h>
#include <token.h>
#include <pp2_macro.h>
#include <preprocess_2.h>
#include <lexer.h>


void macro_def(Preprocessor2* pp2) {
  MacroDef def = {0};
  def.name = expect_token_kind(pp2->token_source, TOK_IDENT).ident; // name
  Token token = next_token(pp2->token_source); // either LPAREN (functionlike macro) or smth else (objectlike macro)

  // If it is function like macro, parse (argument list)
  if (token.kind == SEP_LPAREN) {
    while (true) {
      token = expect_token_kind(pp2->token_source, TOK_IDENT);
      kv_push(interned_str, def.argnames, token.ident);

      token = next_token(pp2->token_source);
      if (token.kind == SEP_COMMA) continue;
      else if (token.kind == SEP_RPAREN) break;
      else throw_error(pp2->token_source, token, "Unexpected token in macro definition");
    }
  }

  // Parse body, for both function-like & object-like macros
  while (token.kind != TOK_EOF && token.kind != SEP_NEWLINE) {
    token = next_token(pp2->token_source);
    kv_push(Token, def.body, token);
  }
  imap_set(def, pp2->macros, def.name);
}


void objectlike_macro_use(Preprocessor2* pp2, MacroDef* def) {
  // recursively expand
  TokenSource src = ts_from_array(def->body, str_to_sv(kv_top(pp2->ctx->source_stack)));
  Preprocessor2 child = new_pp2(pp2->ctx, &src);
  TokenArray result = resolve_pp2(&child);
  
  for (size_t i=0; i<kv_size(result); i++) {
    kv_push(Token, pp2->stream, kv_A(result, i));
  }
  
  kv_destroy(result);
}


void parse_functionlikemacro_call_args(Preprocessor2* pp2, internedmap_t(MacroArgBody)* args, MacroDef* def) {
  MacroArgBody arg = {0};
  Token token = next_token(pp2->token_source);
  size_t arg_num = 0;
  
  while (token.kind != SEP_RPAREN) {
    if (token.kind == TOK_EOF) throw_error(pp2->token_source, token, "Unexpected EOF during macro call.");
    else if (token.kind == SEP_COMMA) {
      assert(arg_num <= kv_size(def->argnames));
      imap_set(arg, (*args), kv_A(def->argnames, arg_num));
      kv_init(arg); // reset arg
      arg_num++;
    }
    else {
      kv_push(Token, arg, token);
    }
    token = next_token(pp2->token_source);
  }
}

bool is_ident_macro_arg(Preprocessor2* pp2, interned_str ident, MacroDef* def) {
  for (size_t i = 0; i < kv_size(def->argnames); i++) {
    interned_str arg = kv_A(def->argnames, i);
    if (interned_eq(arg, ident)) return true;
  }
  return false;
}


void expand_functionlike_macro_body(Preprocessor2* pp2, MacroDef* def, internedmap_t(MacroArgBody)* args, kvec_t(Token)* expanded) {
  for (size_t i=0; i<kv_size(def->body); i++) {
    Token token = kv_A(def->body, i);
    // If the body has a reference to an argumentname, replace argumentname with argumentbody
    if (token.kind == TOK_IDENT && is_ident_macro_arg(pp2, token.ident, def)) {
      MacroArgBody* arg = imap_get((*args), token.ident);
      for (size_t i=0; i<kv_size(*arg); i++) kv_push(Token, pp2->stream, kv_A(*arg, i));
    }
    // else just push
    else kv_push(Token, *expanded, token);
  }
}

void functionlike_macro_use(Preprocessor2* pp2, MacroDef* def) {
  expect_token_kind(pp2->token_source, SEP_LPAREN);

  // Parse arguments
  internedmap_t(MacroArgBody) args;
  parse_functionlikemacro_call_args(pp2, (void*)&args, def);

  // Expand body
  TokenArray expanded = {0};
  expand_functionlike_macro_body(pp2, def, (void*)&args, (void*)&expanded);

  // Recursively check expanded body for macro use
  TokenSource src = ts_from_array(expanded, str_to_sv(kv_top(pp2->ctx->source_stack)));
  Preprocessor2 child = new_pp2(pp2->ctx, &src);
  TokenArray result = resolve_pp2(&child);

  // Push result to preprocessor stream
  for (size_t i=0; i<kv_size(result); i++) {
    kv_push(Token, pp2->stream, kv_A(result, i));
  }
  kv_destroy(expanded);
  kv_destroy(result);
}

void macro_use(Preprocessor2* pp2, Token* usage_tok) {
  MacroDef* def = imap_get(pp2->macros, usage_tok->ident);
  if (!def) return;
  // Object-like macros
  if (kv_size(def->argnames) == 0)
    objectlike_macro_use(pp2, def);
  // Function-like
  else functionlike_macro_use(pp2, def);
}
