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
  NOTE: For my short term sanity, im restricting Token pasting to just identifiers, will add it later if i wanna get cracked
*/
#include <cursor.h>
#include <da_arena.h>
#include <compiler.h>
#include <memscope.h>
#include <da_string.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
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
  printf("Macro %.*s", def->name.len, def->name.cstr);
  if (def->is_functionlike) {
    printf("(");
    kv_foreach(interned_str, def->argnames, i, arg) {
      printf("%.*s", arg.len, arg.cstr);
      if (i != def->argnames.n-1) printf(", ");
    }
    printf("): ");
  }
  else
    printf(": ");
  print_token_array(&def->body);
}

bool are_tokens_adjacent(Token t1, Token t2) {
  return (t1.pos.line == t2.pos.line && t1.pos.id + t1.len == t2.pos.id);
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

  if (imap_has(pp2->ctx->macros, name.ident)) {
    MacroDef* olddef = imap_get(pp2->ctx->macros, name.ident);
    free_macro_def(olddef);
  }
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
  TokenSource src = ts_from_array(*arg);
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
      // >= instead of > because we want argnum + 1 <= no of args
      if (argnum >= kv_size(def->argnames)) throw_error(pp2->token_source, token, "Excess argumments passed to fnlike macro", pp2->ctx);
      push_fnlike_arg(pp2, &arg, args, kv_A(def->argnames, argnum++));
    }
    else if (token.kind == SEP_RPAREN) {
      if (argnum == 0 && kv_size(def->argnames) == 0) {
        if (arg.n == 0) break;
        else throw_error(pp2->token_source, token, "Number of passed and needed arguments dont match", pp2->ctx);
      }
      else if (argnum + 1 != kv_size(def->argnames)) throw_error(pp2->token_source, token, "Number of passed and needed arguments dont match", pp2->ctx);
      push_fnlike_arg(pp2, &arg, args, kv_A(def->argnames, argnum++));
      break;
    }
    else if (token.kind == TOK_EOF) throw_error(pp2->token_source, token, "Unexpected EOF", pp2->ctx);
    else kv_push(Token, arg, token);
    token = next_token(pp2->token_source);
  }
  pop_memscope(pp2->ctx);
}

bool is_token_arg(MacroCallArgMap* args, MacroDef* def, Token token) {
  return imap_has(*args, token.ident) && token.kind == TOK_IDENT;
}

void check_and_substitute_arg(MacroCallArgMap* args, MacroDef* def, TokenArray* expanded, Token token) {
  if (is_token_arg(args, def, token)) {
    kv_push_vec(Token, *expanded, *imap_get(*args, token.ident));
    return;
  }
  kv_push(Token, *expanded, token);
}

bool check_and_join_tokens(Preprocessor2* pp2, MacroCallArgMap* args, TokenArray* body, TokenArray* expanded, size_t* tokid, Token operand1) {
  if (*tokid + 2 >= kv_size(*body)) return false;
  
  Token next = kv_A(*body, *tokid + 1);
  if (next.kind != OP_TOKEN_PASTE) return false;
  Token operand2 = kv_A(*body, *tokid + 2);
  if (operand2.kind != TOK_IDENT) return false;

  interned_str pasted = concat_intern(pp2->ctx->table, interned_to_sv(operand1.ident), interned_to_sv(operand2.ident));
  string pastedstr = { pasted.cstr, pasted.len };
  kv_push(string, pp2->ctx->source_stack, pastedstr);
  Lexer lexer = new_lexer(pp2->ctx);
  Token new = lex_next_token(&lexer); // Token pasting only results in 1 token
  kv_pop(pp2->ctx->source_stack);
  *tokid += 2;
  if (!check_and_join_tokens(pp2, args, body, expanded, tokid, new)) {
    kv_push(Token, *expanded, new);
  }
  return true;
}

void stringify(Preprocessor2* pp2, MacroCallArgMap* args, MacroDef* def, TokenArray* expanded, size_t* i) {
  if (*i+1 >= def->body.n)
      throw_error(pp2->token_source, kv_A(def->body, *i), "Stringification operator followed by unexpected EOF", pp2->ctx);
  Token argtok = kv_A(def->body, *i+1);
  if (argtok.kind != TOK_IDENT)
      throw_error(pp2->token_source, kv_A(def->body, *i+1), "Stringification operator followed by unexpected token", pp2->ctx);
  if (!imap_has(*args, argtok.ident))
      throw_error(pp2->token_source, kv_A(def->body, *i+1), "Stringification operator must be followed by a macro parameter", pp2->ctx);
  TokenArray* arg = imap_get(*args, argtok.ident);
  if (arg->n == 0) {
    kv_push(Token, *expanded, new_token(argtok.pos, sv_from_cstr("\"\""), 2));
  }
  else {
    Token first = kv_A(*arg, 0);
    size_t sum = 0;
    size_t n = 0;
    da_string* buf = &pp2->ctx->buf;
    reset_ds(buf);
    push_char_ds(buf, '\"');
    while (n < arg->n)
      push_ds(buf, token_to_str(&kv_A(*arg, n++)));
    push_char_ds(buf, '\"');
    string str = new_str(arena_alloc(pp2->ctx->arena, buf->len), buf->len);
    memcpy((void*)str.cstr, get_cstr_from_ds(buf), buf->len);
    kv_push(Token, *expanded, new_token(argtok.pos, str_to_sv(str), str.len));
    reset_ds(buf);
  }
  *i += 1;
}

void macro_use_fnlike_body(Preprocessor2* pp2, MacroCallArgMap* args, TokenArray* expanded, MacroDef* def) {
  kv_foreach(Token, def->body, i, token) {
    if (token.kind == TOK_IDENT)
      check_and_substitute_arg(args, def, expanded, token);
    else if (token.kind == OP_PREPROCESS)
      stringify(pp2, args, def, expanded, &i);
    else kv_push(Token, *expanded, token);
  }
  TokenArray buf = {0};
  kv_foreach(Token, *expanded, i, token) {
    if (token.kind == TOK_IDENT && !check_and_join_tokens(pp2, args, expanded, &buf, &i, token))
      kv_push(Token, buf, token);
    else if (token.kind != TOK_IDENT)
      kv_push(Token, buf, token);
  }
  kv_destroy(*expanded);
  *expanded = buf;
}

void macro_use_fnlike(Preprocessor2* pp2, MacroDef* def, TokenArray* out) {
  MemScope* scope = new_scope();
  push_memscope(pp2->ctx, scope);

  MacroCallArgMap args;
  track_mem(scope, &args, (void*)free_macro_call_arg_map);
  imap_init(args);

  TokenArray buf = {0};
  track_mem(scope, &buf, (void*)free_token_array);

  macro_use_fnlike_args(pp2, def, &args);

  // Cyclic macro check
  kv_push(interned_str, pp2->ctx->macro_stack, def->name);
  macro_use_fnlike_body(pp2, &args, &buf, def);

  TokenSource src = ts_from_array(buf);
  TokenArray result = recursively_expand(pp2, &src);
  kv_pop(pp2->ctx->macro_stack);
  track_mem(scope, &result, (void*)free_token_array);

  kv_push_vec(Token, *out, result);
  
  pop_memscope(pp2->ctx);
}

void macro_use_objlike(Preprocessor2* pp2, MacroDef* def, TokenArray* out) {
  kv_push(interned_str, pp2->ctx->macro_stack, def->name);
  TokenSource src = ts_from_array(def->body);
  TokenArray result = recursively_expand(pp2, &src);
  kv_pop(pp2->ctx->macro_stack);

  kv_push_vec(Token, *out, result);
  free_token_array(&result);
}

bool check_cyclic_macros(Preprocessor2* pp2, interned_str name) {
  kv_foreach(interned_str, pp2->ctx->macro_stack, i, macro) {
    if (interned_eq(macro, name)) return true;
  }
  return false;
}

void macro_use(Preprocessor2* pp2, Token* token, TokenArray* out) {
  MacroDef* def = imap_get(pp2->ctx->macros, token->ident);
  if (!def) return;
  if (check_cyclic_macros(pp2, token->ident)) {
    kv_push(Token, *out, *token);
    return;
  }

  if (def->is_functionlike && peek_token(pp2->token_source).kind == SEP_LPAREN)
    macro_use_fnlike(pp2, def, out);
  else if (!def->is_functionlike)
    macro_use_objlike(pp2, def, out);
  else
    kv_push(Token, *out, *token);
}

void free_macro_def(MacroDef *def) {
  kv_destroy(def->argnames);
  kv_destroy(def->body);
}
