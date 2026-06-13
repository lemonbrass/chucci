#include "frontend/cursor.h"
#include <frontend/token.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <utils/string.h>
#include <utils/string_interner.h>

const char *tok_to_str[__token_kind_count] = {
#define X(a, b) b,
    KEYWORDS(X)
#undef X
#define X(a, b, c) b,
        OPERATORS(X)
#undef X
#define X(a, b, c) b,
            SEPARATORS(X)
#undef X
                "eof",
    "ident", "value"};
#define X(a, b, c) [(unsigned char) c] = true,
const bool is_op_table[256] = {OPERATORS(X)};
const bool is_sep_table[256] = {SEPARATORS(X)};
#undef X

StringView token_to_str(Token *token) { return span_to_sv(token->span); }

Token new_tok_ident(Span span, StringID name) {
  Token token = {0};
  token.kind = TOK_IDENT;
  token.ident = name;
  token.span = span;
  return token;
}

Token new_tok_val(Span span) {
  Token token = {0};
  token.kind = TOK_VAL;
  token.span = span;
  return token;
}

Token new_tok_simple(Span span, TokenKind kind) {
  Token token;
  token.kind = kind;
  token.span = span;
  return token;
}

void print_token(Token *token) {
  StringView str = span_to_sv(token->span);
  printf("%s: ", tok_to_str[token->kind]);
  anystr_print(str);
}
void print_token_pretty(Token *token) {
  if (token->kind == SEP_NEWLINE) {
    printf("sep(\\n)");
    return;
  }
  switch (token->kind) {
#define X(a, b, c)                                                             \
  case a:                                                                      \
    printf("op(%s)", b);                                                       \
    break;
    OPERATORS(X)
#undef X
#define X(a, b, c)                                                             \
  case a:                                                                      \
    printf("sep(%s)", b);                                                      \
    break;
    SEPARATORS(X)
#undef X
#define X(a, b)                                                                \
  case a:                                                                      \
    printf("keyword(%s)", b);                                                  \
    break;
    KEYWORDS(X)
#undef X
  case TOK_EOF:
    printf("eof");
    break;
  case TOK_VAL:
    printf("val(");
    anystr_print(token_to_str(token));
    printf(")");
    break;
  case TOK_IDENT:
    (void)0; // To shut up the warning
    StringView str = token_to_str(token);
    printf("ident(%.*s)", (int)str.len, str.cstr);
    break;
  default:
    assert(false && "UNREACHABLE");
  }
}
