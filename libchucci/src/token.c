#include <da_string.h>
#include <stdbool.h>
#include <token.h>

const char* tok_to_str[__token_kind_count] = {
  #define X(a, b, c) b,
  OPERATORS(X)
  #undef X
  #define X(a, b) b,
  KEYWORDS(X)
  #undef X
  #define X(a, b, c) b,
  SEPERATORS(X)
  #undef X
  "eof", "error", "ident", "num"
};
#define X(a, b, c) [(unsigned char)c] = true,
bool is_op_table[256] = {
  OPERATORS(X)
};
#undef X

Token new_tok_ident(CursorMark pos, interned_str name) {
  Token token;
  token.kind = TOK_IDENT;
  token.pos = pos;
  token.ident = name;
  return token;
}

Token new_tok_error(CursorMark pos, int c_line, const char* c_file, const char* error) {
  Token token;
  token.kind = TOK_ERROR;
  token.pos = pos;
  token.error = (TokenError) { .str=error, .c_file=c_file, .c_line=c_line };
  return token;
}

Token new_tok_num(CursorMark pos, string_view num) {
  Token token;
  token.kind = TOK_NUM;
  token.pos = pos;
  token.num = num;
  return token;
}

Token new_tok_simple(CursorMark pos, TokenKind kind) {
  Token token;
  token.kind = kind;
  token.pos = pos;
  return token;
}

void print_token(Token* token) {
  switch (token->kind) {
    #define X(a, b, c) case a: printf("op(%s)", b); break;
     OPERATORS(X)
    #undef X
    #define X(a, b, c) case a: printf("sep(%s)", b); break;
     SEPERATORS(X)
    #undef X
    #define X(a, b) case a: printf("keyword(%s)", b); break;
     KEYWORDS(X)
    #undef X
    case TOK_EOF:   printf("eof"); break;
    case TOK_ERROR: printf("error(%s: %s(%d))", token->error.str, token->error.c_file, token->error.c_line); break;
    case TOK_NUM:   printf("num(%.*s)", (int)token->num.len, token->num.cstr); break;
    case TOK_IDENT: printf("ident(%.*s)", (int)token->ident.len, token->ident.cstr); break;
    default: assert(false && "UNREACHABLE");
  }
}
