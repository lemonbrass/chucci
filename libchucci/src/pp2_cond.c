#include <assert.h>
#include <ctype.h>
#include <da_internmap.h>
#include <da_string.h>
#include <pp2_cond.h>
#include <pp2_macro.h>
#include <preprocess_2.h>
#include <stddef.h>
#include <stdio.h>
#include <thirdparty/kvec.h>
#include <token.h>
#include <token_source.h>

int eval_cond(Preprocessor2 *pp2, int minbp);

int get_bp(Preprocessor2 *pp2, Token token) {
#define X(op, b, bp)                                                           \
  if (token.kind == op)                                                        \
    return bp;
  PP_INFIX_OPERATORS(X)
  PP_PREFIX_OPERATORS(X)
  PP_PAREN_OPERATOR(X)
  PP_TERNARY_OPERATOR(X)
#undef X
  throw_error(pp2->token_source, token, "Unexpected token", pp2->ctx);
  assert(false && "UNREACHABLE");
}

// TODO: Segregate string and numerical tokens, and parse string->number in
// lexer
int strntoi(const char *s, size_t n) {
  const char *p = s;
  const char *end = s + n;

  int result = 0;
  int digits = 0;

  while (p < end && isdigit((unsigned char)*p)) {
    result = result * 10 + (*p - '0');
    p++;
    digits++;
  }

  return digits ? result : 0;
}

int nud(Preprocessor2 *pp2, Token token) {
  // Number
  if (token.kind == TOK_VAL && token.val.cstr[0] != '"') {
    return strntoi(token.val.cstr, token.val.len);
  }
  if (token.kind == TOK_IDENT) {
    if (s_eq(token.ident, sv_from_cstr("defined"))) {
      expect_token_kind(pp2->token_source, SEP_LPAREN, pp2->ctx);
      Token macro = expect_token_kind(pp2->token_source, TOK_IDENT, pp2->ctx);
      expect_token_kind(pp2->token_source, SEP_RPAREN, pp2->ctx);
      return imap_has(pp2->ctx->macros, macro.ident);
    }
    if (!imap_has(pp2->ctx->macros, token.ident))
      return 0;
    TokenArray buf = {0};
    macro_use(pp2, &token, &buf);
    TokenSource ts = ts_from_array(buf);

    Preprocessor2 tmp = *pp2;
    tmp.token_source = &ts;
    return eval_cond(&tmp, 0);
  }
#define X(opkind, op, bp)                                                      \
  if (opkind == token.kind)                                                    \
    return op eval_cond(pp2, bp);
  PP_PREFIX_OPERATORS(X)
#undef X
  if (SEP_LPAREN == token.kind) {
    int res = eval_cond(pp2, 0);
    expect_token_kind(pp2->token_source, SEP_RPAREN, pp2->ctx);
    return res;
  }
  throw_error(pp2->token_source, token, "Unexpected token", pp2->ctx);
  assert(false && "UNREACHABLE");
}

int led(Preprocessor2 *pp2, Token token, int left) {
#define X(opkind, op, bp)                                                      \
  if (opkind == token.kind)                                                    \
    return left op eval_cond(pp2, bp);
  PP_INFIX_OPERATORS(X)
#undef X
  if (OP_QUESTION == token.kind) {
    int then = eval_cond(pp2, 0);
    expect_token_kind(pp2->token_source, OP_COLON, pp2->ctx);
    int else_then = eval_cond(pp2, 0);
    return left ? then : else_then;
  }
  throw_error(pp2->token_source, token, "Unexpected token", pp2->ctx);
  assert(false && "UNREACHABLE");
}

int eval_cond(Preprocessor2 *pp2, int minbp) {
  int left = nud(pp2, next_token(pp2->token_source));

  while (true) {
    Token op = peek_token(pp2->token_source);
    if (op.kind == TOK_EOF || op.kind == SEP_RPAREN || op.kind == OP_COLON ||
        op.kind == SEP_NEWLINE)
      break;
    int bp = get_bp(pp2, op);

    if (bp <= minbp)
      break;
    next_token(pp2->token_source); // skip peeked token

    left = led(pp2, op, left);
  }
  return left;
}

void push_body(Preprocessor2 *pp2) {
  Token next;
  bool skip = false;
  TokenArray buf = {0};
  while ((next = next_token(pp2->token_source), true)) {
    if (next.kind == TOK_EOF)
      throw_error(pp2->token_source, next, "Unexpected EOF", pp2->ctx);
    if (next.kind == OP_PREPROCESS) {
      Token op = peek_token(pp2->token_source);
      if (op.kind == TOK_IDENT && s_eq(op.ident, sv_from_cstr("endif"))) {
        next_token(pp2->token_source);
        break;
      }
      if (op.kind == TOK_IDENT && s_eq(op.ident, sv_from_cstr("elif")))
        skip = true;
      if (op.kind == KW_ELSE)
        skip = true;
    }
    if (!skip)
      kv_push(Token, buf, next);
  }
  TokenSource src = ts_from_array(buf);
  TokenArray result_tokens = recursively_expand(pp2, &src);
  kv_push_vec(Token, pp2->stream, result_tokens);
  kv_destroy(buf);
  kv_destroy(result_tokens);
}

void cond_use(Preprocessor2 *pp2) {
  int res = eval_cond(pp2, 0);
  if (res) {
    push_body(pp2);
  } else {
    Token next;
    TokenArray buf = {0};
    bool skip = true;
    while ((next = next_token(pp2->token_source)), true) {
      if (next.kind == TOK_EOF)
        throw_error(pp2->token_source, next, "Unexpected EOF", pp2->ctx);
      if (next.kind == OP_PREPROCESS) {
        Token op = peek_token(pp2->token_source);
        if (op.kind == TOK_IDENT && s_eq(op.ident, sv_from_cstr("endif"))) {
          next_token(pp2->token_source);
          break;
        }
        if (op.kind == TOK_IDENT && s_eq(op.ident, sv_from_cstr("elif"))) {
          next_token(pp2->token_source);
          cond_use(pp2);
          break;
        }
        if (op.kind == KW_ELSE) {
          next_token(pp2->token_source);
          skip = false;
          continue;
        }
      }
      if (!skip)
        kv_push(Token, buf, next);
    }
    TokenSource src = ts_from_array(buf);
    TokenArray result_tokens = recursively_expand(pp2, &src);
    kv_push_vec(Token, pp2->stream, result_tokens);
    kv_destroy(buf);
    kv_destroy(result_tokens);
  }
}
