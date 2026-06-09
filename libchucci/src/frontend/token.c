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

StringView token_to_str(Token *token, StringInterner *interner) {
  if (token->kind < TOK_EOF)
    return cstr_to_sv((char *)tok_to_str[token->kind]);
  else if (token->kind == TOK_VAL)
    return token->lexeme;
  else if (token->kind == TOK_IDENT)
    return get_interned_sv(interner, token->ident);
  else if (token->kind == SEP_NEWLINE)
    return const_cstr_to_sv("\n");
  else
    assert(false);
}

Token new_tok_ident(CursorMark pos, StringView lexeme, File *file,
                    StringID name) {
  Token token;
  token.kind = TOK_IDENT;
  token.pos = pos;
  token.ident = name;
  token.file = file;
  return token;
}

Token new_tok_val(CursorMark pos, StringView lexeme, File *file) {
  Token token;
  token.kind = TOK_VAL;
  token.pos = pos;
  token.lexeme = lexeme;
  token.file = file;
  return token;
}

Token new_tok_simple(CursorMark pos, StringView lexeme, File *file,
                     TokenKind kind) {
  Token token;
  token.kind = kind;
  token.pos = pos;
  token.lexeme = cstr_to_sv((char *)tok_to_str[kind]);
  token.file = file;
  return token;
}

void print_token(Token *token, StringInterner *interner) {
  StringView str = token_to_str(token, interner);
  printf("%.*s at (%zu, %zu)", (int)str.len, str.cstr, token->pos.col,
         token->pos.line);
}
void print_token_pretty(Token *token, StringInterner *interner) {
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
    printf("val(%.*s)", (int)token->lexeme.len, token->lexeme.cstr);
    break;
  case TOK_IDENT:
    (void)0; // To shut up the warning
    StringView str = token_to_str(token, interner);
    printf("ident(%.*s)", (int)str.len, str.cstr);
    break;
  default:
    assert(false && "UNREACHABLE");
  }
}
