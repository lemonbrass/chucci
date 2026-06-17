#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "frontend/cursor.h"
#include "frontend/pp_macro.h"
#include "frontend/preprocessor.h"
#include "frontend/token.h"
#include "frontend/token_stream.h"
#include "utils/diagnostics.h"
#include "utils/linear_map.h"
#include "utils/string.h"
#include "utils/string_interner.h"
#include "utils/vmem_arena.h"

LINEAR_MAP_IMPL(StringID, MacroDef, MacroDefMap, macrodefmap,
                VMEM_ARENA_ALLOC_INT)
#define stack(pp) &(pp)->streams

void macrodef_free(MacroDef* def, CompilerCtx* ctx) {
  stridvec_free(&def->args, ctx->arena);
  tokenvec_free(&def->body, ctx->arena);
}

void print_macro_def(Preprocessor* pp, CompilerCtx* ctx, MacroDef* def) {
  printf("DEF{\n");
  StringView name = span_to_sv(def->name.span);
  printf("  ");
  anystr_print(name);
  if (def->is_fnlike) {
    putchar('(');
    for (size_t i = 0; i < def->args.len; i++) {
      StringView sv =
          get_interned_sv(ctx->interner, stridvec_access(&def->args, i));
      anystr_print(sv);
      if (i < def->args.len - 1) printf(", ");
    }
    putchar(')');
  }
  printf(": ");
  for (size_t i = 0; i < def->body.len; i++) {
    print_token_pretty(tokenvec_access_ptr(&def->body, i));
    putchar(' ');
  }
  putchar('\n');
  printf("}\n");
}

void macro_def_args(Preprocessor* pp, CompilerCtx* ctx, MacroDef* def,
                    bool* has_error) {
  Token next = ts_stack_next_token(stack(pp), ctx);
  bool expect_comma = false;
  while (next.kind != SEP_RPAREN && next.kind != TOK_EOF) {
    if (next.kind == TOK_IDENT) {
      stridvec_push(&def->args, next.ident, ctx->arena);
      expect_comma = true;
    } else if (next.kind == SEP_COMMA && expect_comma) {
      expect_comma = false;
    } else {
      *has_error = true;
      diagnostic_new(ctx->engine, next.span, ERR_INVALID_MACRO_DEF);
    }
    next = ts_stack_next_token(stack(pp), ctx);
  }
  if (next.kind != SEP_RPAREN) {
    *has_error = true;
    diagnostic_new(ctx->engine, next.span, ERR_INVALID_MACRO_DEF);
  }
}

void macro_def(Preprocessor* pp, CompilerCtx* ctx, Token token) {
  MacroDef def = {0};
  def.is_expanding = false;
  def.name = ts_stack_expect_token(stack(pp), ctx, TOK_IDENT);
  Token next = ts_stack_peek_token(stack(pp), ctx);
  bool has_error = def.name.kind != TOK_IDENT;
  // Function Like Macro
  if (next.kind == SEP_LPAREN &&
      def.name.span.start + def.name.span.len == next.span.start) {
    def.is_fnlike = true;
    next = ts_stack_next_token(stack(pp), ctx);  // skip (

    macro_def_args(pp, ctx, &def, &has_error);
  }
  def.body = tokenvec_new();
  def.span.start = next.span.start;
  while (next.kind != SEP_NEWLINE && next.kind != TOK_EOF) {
    next = ts_stack_next_token(stack(pp), ctx);
    if (!has_error) tokenvec_push(&def.body, next, ctx->arena);
  }
  def.span.len = next.span.start - def.span.start + next.span.len;
  if (has_error) {
    macrodef_free(&def, ctx);
    return;
  }
  macrodefmap_set(&pp->macros, def.name.ident, def, ctx->arena);
}

void note_macro_defined_here(DIAGID diag, CompilerCtx* ctx, MacroDef* def) {
  DIAGID subdiag = subdiagnostic_new(diag, ctx->engine, def->name.span,
                                     NOTE_MACRO_DEFINED_HERE);
  CursorMark mark = span_start(def->name.span);
  subdiag_set_format(ctx->engine, subdiag,
                     "%[file] (%[row]:%[col]): %[level]: %[msg]\n"
                     "%[4]\n");
  subdiag_add_arg(ctx->engine, subdiag,
                  diagarg_sv(span_to_sv(def->name.span)));  // name
  subdiag_add_arg(
      ctx->engine, subdiag,
      diagarg_sv(def->name.span.source->name));  // file where macro is declared
  subdiag_add_arg(ctx->engine, subdiag, diagarg_num(mark.line));  // row
  subdiag_add_arg(ctx->engine, subdiag, diagarg_num(mark.col));   // col
  subdiag_add_arg(ctx->engine, subdiag,
                  diagarg_sv(span_curr_line(def->name.span)));
}

DIAGID fnlike_macro_use_err(DiagnosticID id, MacroDef* def, Token next,
                            CompilerCtx* ctx) {
  DIAGID diag = diagnostic_new(ctx->engine, next.span, id);
  diag_set_format(ctx->engine, diag,
                  "%[file] (%[row]:%[col]): %[level]: %[msg]\n"
                  "Initiated by %[cfile]:%[cline]\n"
                  "%[span-line]\n"
                  "^%[span-line.len--*~]\n");
  note_macro_defined_here(diag, ctx, def);
  return diag;
}

void match_rparen(Preprocessor* pp, CompilerCtx* ctx, MacroDef* def,
                  MacroArgMap* map, TokenVec* arg) {
  Token next = ts_stack_next_token(&pp->streams, ctx);
  while (next.kind != SEP_RPAREN && next.kind != TOK_EOF) {
    if (next.kind == SEP_LPAREN) {
      tokenvec_push(arg, next, pp->arena);
      match_rparen(pp, ctx, def, map, arg);
    } else {
      tokenvec_push(arg, next, pp->arena);
      next = ts_stack_next_token(&pp->streams, ctx);
    }
  }
  if (next.kind == TOK_EOF) {
    fnlike_macro_use_err(ERR_INVALID_FNLIKE_MACRO_USE, def, next, ctx);
  }
}

MacroArgMap fnlike_macro_use_args(Preprocessor* pp, CompilerCtx* ctx,
                                  Token name, MacroDef* def) {
  MacroArgMap map = macroargmap_new();
  TokenVec arg = tokenvec_new();
  ts_stack_expect_token(&pp->streams, ctx, SEP_LPAREN);  // skip (
  Token next = ts_stack_next_token(&pp->streams, ctx);
  size_t arg_idx = 0;
  bool expect_comma = false;
  if (def->args.len == 0) return map;
  while (next.kind != SEP_RPAREN && next.kind != TOK_EOF) {
    while (next.kind != SEP_COMMA && next.kind != SEP_RPAREN &&
           next.kind != TOK_EOF) {
      if (next.kind == SEP_LPAREN) {
        tokenvec_push(&arg, next, pp->arena);
        match_rparen(pp, ctx, def, &map, &arg);
      } else {
        tokenvec_push(&arg, next, pp->arena);
      }
      next = ts_stack_next_token(&pp->streams, ctx);
    }
    macroargmap_set(&map, stridvec_access(&def->args, arg_idx++), arg,
                    pp->arena);
    arg = tokenvec_new();
    if (next.kind == SEP_COMMA) next = ts_stack_next_token(&pp->streams, ctx);
  }
  if (next.kind != SEP_RPAREN) {
    fnlike_macro_use_err(ERR_INVALID_FNLIKE_MACRO_USE, def, next, ctx);
  }
  if (arg_idx != def->args.len) {
    DIAGID diag =
        fnlike_macro_use_err(ERR_DIFF_FNLIKE_MACRO_ARGS, def, name, ctx);
    diag_add_arg(ctx->engine, diag, diagarg_num(def->args.len));  // expected
    diag_add_arg(ctx->engine, diag, diagarg_num(arg_idx));        // found
  }
  return map;
}

TokenStream macro_use(Preprocessor* pp, CompilerCtx* ctx, Token name,
                      MacroDef* def) {
  if (def->is_expanding) {
    TokenStream ts = ts_from_token(name);
    return ts;
  }
  def->is_expanding = true;
  if (def->is_fnlike) {
    MacroUseStream* mu = vmarena_calloc(pp->arena, sizeof(MacroUseStream));
    mu->arena = pp->arena;
    mu->def = def;
    mu->streams = ts_stack_new();
    mu->argmap = fnlike_macro_use_args(pp, ctx, name, def);
    TokenStream ts = ts_from_vec(mu->def->body);
    ts_stack_push(&mu->streams, ts, pp->arena);
    return ts_from_macro_use(mu);
  } else {
    MacroUseStream* mu = vmarena_calloc(pp->arena, sizeof(MacroUseStream));
    mu->arena = pp->arena;
    mu->def = def;
    mu->streams = ts_stack_new();
    TokenStream ts = ts_from_vec(mu->def->body);
    ts_stack_push(&mu->streams, ts, pp->arena);
    return ts_from_macro_use(mu);
  }
  assert(false);
}

Token mu_stringify_token(TokenStream* ts, CompilerCtx* ctx) {
  Token target = ts_next_token(ts, ctx);
  StringView lexeme = span_to_sv(target.span);
  size_t len = lexeme.len + 2;
  char* cstr = vmarena_alloc(ctx->arena, len);
  cstr[0] = '"';
  cstr[len - 1] = '"';
  memcpy(cstr + 1, lexeme.cstr, lexeme.len);
  StringView str = sv_new(cstr, len);
  return new_tok_val(span_from_sv(ctx->arena, str, 0));
}

Token mu_token_paste(TokenStream* ts, CompilerCtx* ctx, Token token1,
                     Token token2) {
  char buf[1024] = {0};
  StringView token1_str = get_interned_sv(ctx->interner, token1.ident);
  StringView token2_str = get_interned_sv(ctx->interner, token2.ident);
  size_t len = token1_str.len + token2_str.len;
  memcpy(buf, token1_str.cstr, token1_str.len);
  memcpy(buf + token1_str.len, token2_str.cstr, token2_str.len);
  StringID ident = intern(sv_new(buf, len), ctx->interner);
  StringView result = get_interned_sv(ctx->interner, ident);
  Span span = span_from_sv(ctx->arena, result, 0);
  assert(result.cstr == span_to_sv(span).cstr &&
         result.len == span_to_sv(span).len);
  return new_tok_ident(span, ident);
}

Token mu_check_token_paste(TokenStream* ts, CompilerCtx* ctx, Token token) {
  if (token.kind != TOK_IDENT) {
    return token;
  }
  Token next = mu_peek_token(ts, ctx);
  if (next.kind != OP_TOKEN_PASTE) {
    return token;
  }
  mu_next_token(ts, ctx);  // skip ##
  Token token2 = mu_next_token(ts, ctx);
  if (token2.kind != TOK_IDENT) {
    unexpected_token_err(ctx, token2, TOK_IDENT);
  }
  return mu_token_paste(ts, ctx, token, token2);
}

Token mu_fnlike_next_token(TokenStream* ts, CompilerCtx* ctx) {
  MacroUseStream* mu = ts->macro_use;
  Token token = ts_stack_next_token(&mu->streams, ctx);

  // If the top of the stream is an argument, send it without intercepting
  // for arguments
  if (mu->_is_arg) {
    if (mu->streams.len == 1) {
      mu->_is_arg = false;
      return mu_check_token_paste(ts, ctx, token);
    }
    return token;
  }
  // If token is a newline, macro has been consumed
  if (token.kind == SEP_NEWLINE) {
    ts->is_consumed = true;
    vmarena_reset(mu->arena);
    return token;
  }
  // If we find a token with type IDENT and with content same as that of a
  // parameter, intercept the token, check if it is an argument and replace it
  // with the argument body
  // (by creating a new stream and pushing it at top of stack)
  TokenVec* arg = NULL;
  if (token.kind == TOK_IDENT &&
      (arg = macroargmap_get(&mu->argmap, token.ident))) {
    TokenStream ts_arg = ts_from_vec(*arg);
    ts_stack_push(&mu->streams, ts_arg, ctx->arena);
    mu->_is_arg = true;
    return mu_next_token(ts, ctx);
  }

  // Stringify
  if (token.kind == OP_PREPROCESS) return mu_stringify_token(ts, ctx);

  // Returns the input token if token pasting isnt detected
  // else, token pastes and returns token
  return mu_check_token_paste(ts, ctx, token);
}

Token mu_next_token(TokenStream* ts, CompilerCtx* ctx) {
  MacroUseStream* mu = ts->macro_use;
  if (mu->def->is_fnlike) {
    return mu_fnlike_next_token(ts, ctx);
  } else {
    Token token = ts_stack_next_token(&mu->streams, ctx);
    if (token.kind == SEP_NEWLINE) ts->is_consumed = true;
    return token;
  }
}

// NOTE: Adding a pointer to MacroUseStream might affect this function
// This MUST deep copy MacroUse to its previous state
Token mu_peek_token(TokenStream* ts, CompilerCtx* ctx) {
  TokenStream ts_bak = *ts;
  MacroUseStream mu_bak = *ts->macro_use;
  Token token = mu_next_token(ts, ctx);
  *ts = ts_bak;
  *ts->macro_use = mu_bak;
  return token;
}
