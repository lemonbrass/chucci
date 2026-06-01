#include <assert.h>
#include <ctype.h>
#include <frontend/pp2_cond.h>
#include <frontend/pp2_macro.h>
#include <frontend/preprocess_2.h>
#include <frontend/token.h>
#include <frontend/token_source.h>
#include <stddef.h>
#include <stdio.h>
#include <thirdparty/kvec.h>
#include <utils/da_internmap.h>
#include <utils/da_string.h>

int eval_cond(Preprocessor2 *pp2, int minbp);

#define GLOBAL_CTX(p) ((p)->ctx->global)

int get_bp(Preprocessor2 *pp2, Token token) {
#define X(op, b, bp)                                                           \
  if (token.kind == op)                                                        \
    return bp;
  PP_INFIX_OPERATORS(X)
  PP_PREFIX_OPERATORS(X)
  PP_PAREN_OPERATOR(X)
  PP_TERNARY_OPERATOR(X)
#undef X

  throw_error(pp2->token_source, token, "Unexpected token", GLOBAL_CTX(pp2));
}

int strntoi(const char *s, size_t n) {
  const char *p = s;
  const char *end = s + n;
  int result = 0;
  bool has_digits = false;

  while (p < end && isdigit((unsigned char)*p)) {
    result = result * 10 + (*p - '0');
    p++;
    has_digits = true;
  }

  return has_digits ? result : 0;
}

int nud(Preprocessor2 *pp2, Token token) {
  // Numeric Literals
  if (token.kind == TOK_VAL && token.val.cstr[0] != '"') {
    return strntoi(token.val.cstr, token.val.len);
  }

  // Identifiers / Macros
  if (token.kind == TOK_IDENT) {
    if (s_eq(token.ident, sv_from_cstr("defined"))) {
      expect_token_kind(pp2->token_source, SEP_LPAREN, GLOBAL_CTX(pp2));
      Token macro =
          expect_token_kind(pp2->token_source, TOK_IDENT, GLOBAL_CTX(pp2));
      expect_token_kind(pp2->token_source, SEP_RPAREN, GLOBAL_CTX(pp2));
      return imap_has(pp2->ctx->macros, macro.ident);
    }

    if (!imap_has(pp2->ctx->macros, token.ident)) {
      return 0;
    }

    TokenArray buf = {0};
    macro_use(pp2, &token, &buf);
    TokenSource ts = ts_from_array(buf);

    Preprocessor2 tmp = *pp2;
    tmp.token_source = &ts;
    return eval_cond(&tmp, 0);
  }

  // Prefix Operators
#define X(opkind, op, bp)                                                      \
  if (opkind == token.kind)                                                    \
    return op eval_cond(pp2, bp);
  PP_PREFIX_OPERATORS(X)
#undef X

  // Sub-Expressions
  if (token.kind == SEP_LPAREN) {
    int res = eval_cond(pp2, 0);
    expect_token_kind(pp2->token_source, SEP_RPAREN, GLOBAL_CTX(pp2));
    return res;
  }

  throw_error(pp2->token_source, token, "Unexpected token", GLOBAL_CTX(pp2));
}

int led(Preprocessor2 *pp2, Token token, int left) {
  // Infix Operators
#define X(opkind, op, bp)                                                      \
  if (opkind == token.kind)                                                    \
    return left op eval_cond(pp2, bp);
  PP_INFIX_OPERATORS(X)
#undef X

  // Ternary Operator
  if (token.kind == OP_QUESTION) {
    int then_branch = eval_cond(pp2, 0);
    expect_token_kind(pp2->token_source, OP_COLON, GLOBAL_CTX(pp2));
    int else_branch = eval_cond(pp2, 0);
    return left ? then_branch : else_branch;
  }

  throw_error(pp2->token_source, token, "Unexpected token", GLOBAL_CTX(pp2));
}

int eval_cond(Preprocessor2 *pp2, int minbp) {
  int left = nud(pp2, next_token(pp2->token_source));

  while (true) {
    Token op = peek_token(pp2->token_source);
    if (op.kind == TOK_EOF || op.kind == SEP_RPAREN || op.kind == OP_COLON ||
        op.kind == SEP_NEWLINE) {
      break;
    }

    int bp = get_bp(pp2, op);
    if (bp <= minbp) {
      break;
    }

    next_token(pp2->token_source);
    left = led(pp2, op, left);
  }
  return left;
}
