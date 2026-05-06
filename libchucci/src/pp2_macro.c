/*
  This is a component of preprocessor that handles macros.
  If we find #define (in preprocess2.c:step_pp2) => macro_def is called
  macro_def: adds macro definition to pp2->macros (interned_map which maps macro name identifier to MacroDef)

  If we find an identifier which is in pp2->macros, we run macro_use.
  macro_use:
  checks for cyclic defines, if found, continue without expanding.
  otherwise either run functionlike_macro_use OR objectlike_macro_use

  objectlike_macro_use: it replaces the macro name with the macro body, after a recursive expansion
  of macro body.

  function_like_macro_use:
  it first parses the argument list and recursively expands each argument (in parse_functionlikemacro_call_args).
  then it substitutes all arguments in body (using another interned_map: MacroCallArgMap) in expand_functionlike_macro_body
  then we recursively expand the substituted macro body and free the MacroCallArgMap.

  NOTE: This preprocessor ISNT standard compliant, its a superset of a subset, so we will have better macros too, later,
   but these macros are just here as a challenge
*/
#include <da_string.h>
#include <token_source.h>
#include <assert.h>
#include <da_internmap.h>
#include <da_intern.h>
#include <thirdparty/kvec.h>
#include <token.h>
#include <pp2_macro.h>
#include <preprocess_2.h>
#include <lexer.h>

bool are_tokens_adjacent(Token t1, Token t2) {
  return (t1.pos.line == t2.pos.line && t1.pos.id + get_token_len(t1) == t2.pos.id);
}

void macro_def(Preprocessor2* pp2) {
  MacroDef def = {0};
  Token name = expect_token_kind(pp2->token_source, TOK_IDENT, pp2->ctx);
  def.name = name.ident;
  Token token = next_token(pp2->token_source); // either LPAREN (functionlike macro) or smth else (objectlike macro)
  def.is_functionlike = false;

  /*
    RULE: #define Foo(x) fn(x) => FNLIKE
    BUT   #define Foo (x) fn(x) => OBJECTLIKE (because of that space in between Foo and (x)) => expands to (x) fn(x)
  */
  // If it is function like macro, parse (argument list)
  if (token.kind == SEP_LPAREN && are_tokens_adjacent(name, token)) {
    def.is_functionlike = true;
    while (true) {
      token = next_token(pp2->token_source);
      if (token.kind == TOK_IDENT)
        kv_push(interned_str, def.argnames, token.ident);
      else if (token.kind == SEP_RPAREN)
        break;
      else throw_error(pp2->token_source, token, "Unexpected token in macro definition", pp2->ctx);

      token = next_token(pp2->token_source);
      if (token.kind == SEP_COMMA) continue;
      else if (token.kind == SEP_RPAREN) {
        token = next_token(pp2->token_source);
        break;
      }
      else throw_error(pp2->token_source, token, "Unexpected token in macro definition", pp2->ctx);
    }
  }

  // Parse body, for both function-like & object-like macros
  while (token.kind != TOK_EOF && token.kind != SEP_NEWLINE) {
    kv_push(Token, def.body, token);
    token = next_token(pp2->token_source);
  }
  imap_set(def, pp2->ctx->macros, def.name);
}


void objectlike_macro_use(Preprocessor2* pp2, MacroDef* def) {
  // Cyclicity prevention
  kv_push(interned_str, pp2->ctx->macro_stack, def->name);
  // recursively expand
  TokenSource src = ts_from_array(def->body, str_to_sv(kv_top(pp2->ctx->source_stack)));
  TokenArray result = recursively_expand(pp2, &src);
  kv_push_vec(Token, pp2->stream, result);

  kv_pop(pp2->ctx->macro_stack);
  kv_destroy(result);
}

void destroy_macro_call_arg(TokenArray* arg) {
  kv_destroy(*arg);
}


void parse_functionlikemacro_call_args(Preprocessor2* pp2, MacroCallArgMap* args, MacroDef* def) {
  TokenArray arg = {0};
  imap_init(*args);
  Token token = expect_token_kind(pp2->token_source, SEP_LPAREN, pp2->ctx);
  size_t arg_num = 0;
  size_t depth = 1;
  
  token = next_token(pp2->token_source);
  while (true) {
    if (token.kind == TOK_EOF) throw_error(pp2->token_source, token, "Unexpected EOF during macro call.", pp2->ctx);
    else if (token.kind == SEP_LPAREN) {
      depth++;
      kv_push(Token, arg, token);
    }
    else if ((token.kind == SEP_COMMA || token.kind == SEP_RPAREN) && depth==1) {
      if (kv_size(def->argnames) && arg_num == 0 && token.kind == SEP_RPAREN) {
        token = next_token(pp2->token_source);
        break;
      }
      if(arg_num >= kv_size(def->argnames)) {
        throw_error(pp2->token_source, token, "Too many arguments in macro call.", pp2->ctx);
      }
      
      // Before inserting arg into the map, expand arguments recursively
      TokenSource src = ts_from_array(arg, str_to_sv(kv_top(pp2->ctx->source_stack)));
      TokenArray result = recursively_expand(pp2, &src);
      interned_str argname = kv_A(def->argnames, arg_num); 

      imap_set(result, *args, argname);
      arg_num++;

      kv_destroy(arg);
      arg = (TokenArray){0}; // reset arg
    }
    else {
      kv_push(Token, arg, token);
    }
    if (token.kind == SEP_RPAREN && --depth==0) {
      if (arg_num != kv_size(def->argnames)) {
        imap_destroy(*args, destroy_macro_call_arg);
        throw_error(pp2->token_source, token, "Insufficient amount of arguments in macro call", pp2->ctx);
      }
      break;
    }
    token = next_token(pp2->token_source);
  }
}

bool is_ident_macro_arg(interned_str ident, MacroDef* def) {
  kv_foreach(interned_str, def->argnames, i, arg) {
    if (interned_eq(arg, ident)) return true;
  }
  return false;
}


void expand_functionlike_macro_body(Preprocessor2* pp2, MacroDef* def, MacroCallArgMap* args, TokenArray* expanded) {
  for (size_t i=0; i<kv_size(def->body); i++) {
    Token token = kv_A(def->body, i);
    // If the body has a reference to an argumentname, replace argumentname with argumentbody
    if (token.kind == TOK_IDENT && is_ident_macro_arg(token.ident, def)) {
      TokenArray* arg = imap_get(*args, token.ident);
      kv_foreach(Token, *arg, _, token) {
        if (token.kind == TOK_EOF) break;
        kv_push(Token, *expanded, token);
      }
    }
    // else just push
    else kv_push(Token, *expanded, token);
  }
}

void functionlike_macro_use(Preprocessor2* pp2, MacroDef* def) {
  // Parse arguments
  MacroCallArgMap args;
  parse_functionlikemacro_call_args(pp2, &args, def);

  // Cyclicity prevention
  kv_push(interned_str, pp2->ctx->macro_stack, def->name);

  // Expand body
  TokenArray expanded = {0};
  expand_functionlike_macro_body(pp2, def, &args, &expanded);

  // Recursively check expanded body for macro use
  TokenSource src = ts_from_array(expanded, str_to_sv(kv_top(pp2->ctx->source_stack)));
  TokenArray result = recursively_expand(pp2, &src);

  // Push result to preprocessor stream
  kv_push_vec(Token, pp2->stream, result);

  kv_pop(pp2->ctx->macro_stack);

  imap_destroy(args, destroy_macro_call_arg);
  kv_destroy(expanded);
  kv_destroy(result);
}

bool check_cyclic_macro(Preprocessor2* pp2, interned_str name) {
  for (size_t i=0; i<kv_size(pp2->ctx->macro_stack); i++)
    if (interned_eq(kv_A(pp2->ctx->macro_stack, i), name)) return true;
  return false;
}

void macro_use(Preprocessor2* pp2, Token* usage_tok) {
  MacroDef* def = imap_get(pp2->ctx->macros, usage_tok->ident);
  if (!def) return;
  // Object-like macros
  if (check_cyclic_macro(pp2, def->name)) {
    kv_push(Token, pp2->stream, *usage_tok);
    return;
  }
  Token peeked = peek_token(pp2->token_source);
  if (!def->is_functionlike)
    objectlike_macro_use(pp2, def);
  // Function-like
  else if (peeked.kind == SEP_LPAREN) functionlike_macro_use(pp2, def);
  else kv_push(Token, pp2->stream, *usage_tok);
}

void free_macro_def(MacroDef *def) {
  kv_destroy(def->argnames);
  kv_destroy(def->body);
}
