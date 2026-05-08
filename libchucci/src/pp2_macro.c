/*
  This is a component of preprocessor2 which handles macros.
  macro_def -> define a macro
  mscro_use -> use a macro
  mwcro_use first checks for cyclic macros
  and then either runs macro_use_fnlike or macro_use_objlike
  based on what MacroDef.is_functionlike is.
  mafro_use_objlike -> simply expands the macro body recursively
  until no macros remain and then appends it to stream.
  macro_use_fnlike -> first creates a ArgName (interned_str) -> Arg (TokenArray) mapping.
  Then it expands the macro body after replacing all ArgName references
  with their values acc. to the mapping.
  Lastly it recursively checks for more macros before the result is pushed into the stream.
  NOTE: This preprocessor ISNT standard compliant, its a superset of a subset, so we will have better macros too, later,
   but these macros are just here as a challenge
*/
#include <compiler.h>
#include <memscope.h>
#include <da_string.h>
#include <stddef.h>
#include <token_source.h>
#include <assert.h>
#include <da_internmap.h>
#include <da_intern.h>
#include <thirdparty/kvec.h>
#include <token.h>
#include <pp2_macro.h>
#include <preprocess_2.h>
#include <lexer.h>

void print_macro_def(MacroDef* def) {
  printf("%.*s: ", def->name.len, def->name.cstr);
  if (def->is_functionlike) {
    printf("(");
    kv_foreach(interned_str, def->argnames, i, arg) {
      printf("%.*s", arg.len, arg.cstr);
      if (i != def->argnames.n-1) printf(", ");
    }
    printf(")\n");
  }
  print_token_array(&def->body);
}

bool are_tokens_adjacent(Token t1, Token t2) {
  // return true;
  return (t1.pos.line == t2.pos.line && t1.pos.id + get_token_len(t1) == t2.pos.id);
}

void free_token_array(TokenArray* arr) {
  if (arr && arr->a)
    kv_destroy(*arr);
}

void free_macro_call_arg_map(MacroCallArgMap* args) {
  imap_destroy(*args, free_token_array);
}

void macro_def_parse_args(Preprocessor2* pp2, MacroDef* def) {
  def->is_functionlike = true;
  expect_token_kind(pp2->token_source, SEP_LPAREN, pp2->ctx);
  Token token = next_token(pp2->token_source);
  while (true) {
    if (token.kind == SEP_RPAREN) break; 
    else if (token.kind == TOK_IDENT)
      kv_push(interned_str, def->argnames, token.ident);
    else
      throw_error(pp2->token_source, token, "Unexpected token in macro definition", pp2->ctx);
    
    token = next_token(pp2->token_source);
    
    if (token.kind == SEP_COMMA) {
      token = next_token(pp2->token_source);
      if (token.kind == SEP_RPAREN) 
        throw_error(pp2->token_source, token, "Unexpected token in macro definition", pp2->ctx);
    }
    else if (token.kind == SEP_RPAREN)
      break;
    else
      throw_error(pp2->token_source, token, "Unexpected token in macro definition", pp2->ctx);
  }
}

void macro_def_parse_body(Preprocessor2* pp2, MacroDef* def) {
  Token token = next_token(pp2->token_source);
  while (token.kind != TOK_EOF && token.kind != SEP_NEWLINE) {
    kv_push(Token, def->body, token);
    token = next_token(pp2->token_source);
  }
}

void macro_def(Preprocessor2* pp2) {
  MemScope* scope = new_scope();
  push_memscope(pp2->ctx, scope);
  MacroDef def = {0};
  track_mem(scope, &def, (void*)free_macro_def);
  
  Token name = expect_token_kind(pp2->token_source, TOK_IDENT, pp2->ctx);
  def.name = name.ident;
  Token token = peek_token(pp2->token_source);

  if (token.kind == SEP_LPAREN && are_tokens_adjacent(name, token))
    macro_def_parse_args(pp2, &def);
  macro_def_parse_body(pp2, &def);

  print_macro_def(&def);

  imap_set(def, pp2->ctx->macros, name.ident);
  untrack_mem(scope, &def);
  pop_memscope(pp2->ctx);
}

void fnlike_args_match_paren(Preprocessor2* pp2, TokenArray* arg) {
  Token token = next_token(pp2->token_source);
  while (true) {
    if (token.kind == SEP_LPAREN) {
      kv_push(Token, *arg, token);
      fnlike_args_match_paren(pp2, arg);
    }
    else if (token.kind == SEP_RPAREN) {
      kv_push(Token, *arg, token);
      break;
    }
    else kv_push(Token, *arg, token);
    token = next_token(pp2->token_source);
  }
}

void push_fnlike_arg(Preprocessor2* pp2, TokenArray* arg, MacroCallArgMap* args, interned_str argname) {
  TokenSource src = ts_from_array(*arg, str_to_sv(kv_top(pp2->ctx->source_stack)));
  TokenArray result = recursively_expand(pp2, &src);
  imap_set(result, *args, argname);

  kv_destroy(*arg);
  *arg = (TokenArray){0};
}

void macro_use_fnlike_args(Preprocessor2* pp2, MacroDef* def, MacroCallArgMap* args) {
  MemScope* scope = new_scope();
  push_memscope(pp2->ctx, scope);
  
  expect_token_kind(pp2->token_source, SEP_LPAREN, pp2->ctx);

  TokenArray arg = {0};
  track_mem(scope, &arg, (void*)free_token_array);

  size_t argnum = 0;

  Token token = next_token(pp2->token_source);
  while (true) {
    if (token.kind == SEP_LPAREN) {
      kv_push(Token, arg, token);
      fnlike_args_match_paren(pp2, &arg);
    }
    else if (token.kind == SEP_COMMA) {
      // >= instesd of > because we want argnum + 1 <= no of args
      if (argnum >= kv_size(def->argnames)) throw_error(pp2->token_source, token, "Excess argumments passed to fnlike macro", pp2->ctx);
      if (arg.n > 0)
        push_fnlike_arg(pp2, &arg, args, kv_A(def->argnames, argnum++));
    }
    else if (token.kind == SEP_RPAREN) {
      if (arg.n > 0)
        push_fnlike_arg(pp2, &arg, args, kv_A(def->argnames, argnum++));
      if (argnum != kv_size(def->argnames)) throw_error(pp2->token_source, token, "Number of passed and needed arguments dont match in fnlike macro", pp2->ctx);
      break;
    }
    else if (token.kind == TOK_EOF) throw_error(pp2->token_source, token, "Unexpected EOF", pp2->ctx);
    else kv_push(Token, arg, token);
    token = next_token(pp2->token_source);
  }
  pop_memscope(pp2->ctx);
}

void check_and_substitute_arg(MacroCallArgMap* args, MacroDef* def, TokenArray* expanded, Token token) {
  assert(token.kind == TOK_IDENT);
  kv_foreach(interned_str, def->argnames, i, argname) {
    if (interned_eq(token.ident, argname)) {
      kv_push_vec(Token, *expanded, *imap_get(*args, argname));
      return;
    }
  }
  kv_push(Token, *expanded, token);
}

void macro_use_fnlike_body(Preprocessor2* pp2, MacroCallArgMap* args, TokenArray* expanded, MacroDef* def) {
  kv_foreach(Token, def->body, i, token) {
    if (token.kind == TOK_IDENT) check_and_substitute_arg(args, def, expanded, token);
    else kv_push(Token, *expanded, token);
  }
}

void macro_use_fnlike(Preprocessor2* pp2, MacroDef* def) {
  MemScope* scope = new_scope();
  push_memscope(pp2->ctx, scope);

  MacroCallArgMap args;
  track_mem(scope, &args, (void*)free_macro_call_arg_map);
  imap_init(args);

  pp2->ctx->token_buf.n = 0;

  macro_use_fnlike_args(pp2, def, &args);

  // Cyclic macro check
  kv_push(interned_str, pp2->ctx->macro_stack, def->name);
  macro_use_fnlike_body(pp2, &args, &pp2->ctx->token_buf, def);

  TokenSource src = ts_from_array(pp2->ctx->token_buf, str_to_sv(kv_top(pp2->ctx->source_stack)));
  TokenArray result = recursively_expand(pp2, &src);
  kv_pop(pp2->ctx->macro_stack);
  track_mem(scope, &result, (void*)free_token_array);

  kv_push_vec(Token, pp2->stream, result);
  
  pp2->ctx->token_buf.n = 0;
  pop_memscope(pp2->ctx);
}

void macro_use_objlike(Preprocessor2* pp2, MacroDef* def) {
  kv_push(interned_str, pp2->ctx->macro_stack, def->name);
  TokenSource src = ts_from_array(def->body, str_to_sv(kv_top(pp2->ctx->source_stack)));
  TokenArray result = recursively_expand(pp2, &src);
  kv_pop(pp2->ctx->macro_stack);

  kv_push_vec(Token, pp2->stream, result);
  free_token_array(&result);
}

bool check_cyclic_macros(Preprocessor2* pp2, interned_str name) {
  kv_foreach(interned_str, pp2->ctx->macro_stack, i, macro) {
    if (interned_eq(macro, name)) return true;
  }
  return false;
}

void macro_use(Preprocessor2* pp2, Token* token) {
  MacroDef* def = imap_get(pp2->ctx->macros, token->ident);
  if (!def) return;
  if (check_cyclic_macros(pp2, token->ident)) {
    kv_push(Token, pp2->stream, *token);
    return;
  }

  if (def->is_functionlike)
    macro_use_fnlike(pp2, def);
  else
    macro_use_objlike(pp2, def);
}

void free_macro_def(MacroDef *def) {
  kv_destroy(def->argnames);
  kv_destroy(def->body);
}
