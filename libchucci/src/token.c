#include <da_string.h>
#include <stdbool.h>
#include <stdio.h>
#include <token.h>

const char* tok_to_str[__token_kind_count] = {
  #define X(a, b, c) b,
  OPERATORS(X)
  #undef X
  #define X(a, b) b,
  KEYWORDS(X)
  #undef X
  #define X(a, b, c) b,
  SEPARATORS(X)
  #undef X
  "eof", "error", "ident", "value", "\\n"
};
#define X(a, b, c) [(unsigned char)c] = true,
bool is_op_table[256] = {
  OPERATORS(X)
};
#undef X

size_t get_token_len(Token token) {
  if (token.kind < TOK_EOF) return strlen(tok_to_str[token.kind]);
  else if (token.kind == TOK_EOF) return 1;
  else if (token.kind == TOK_VAL) return token.val.len;
  else if (token.kind == TOK_IDENT) return token.ident.len;
  else if (token.kind == SEP_NEWLINE) return 1;
  else return 0;
}


Token new_tok_ident(Cursor pos, interned_str name) {
  Token token;
  token.kind = TOK_IDENT;
  token.pos = pos;
  token.ident = name;
  return token;
}

Token new_tok_error(Cursor pos, int c_line, const char* c_file, const char* error) {
  Token token;
  token.kind = TOK_ERROR;
  token.pos = pos;
  token.error = (TokenError) { .str=error, .c_file=c_file, .c_line=c_line };
  return token;
}

Token new_tok_val(Cursor pos, string_view val) {
  Token token;
  token.kind = TOK_VAL;
  token.pos = pos;
  token.val = val;
  return token;
}

Token new_tok_simple(Cursor pos, TokenKind kind) {
  Token token;
  token.kind = kind;
  token.pos = pos;
  return token;
}

void print_token(Token* token) {
  switch (token->kind) {
    #define X(a, b, c) case a: printf("op(%s) at (%zu, %zu)", b, token->pos.line, token->pos.col); break;
     OPERATORS(X)
    #undef X
    #define X(a, b, c) case a: printf("sep(%s) at (%zu, %zu)", b, token->pos.line, token->pos.col); break;
     SEPARATORS(X)
    #undef X
    #define X(a, b) case a: printf("keyword(%s) at (%zu, %zu)", b, token->pos.line, token->pos.col); break;
     KEYWORDS(X)
    #undef X
    case TOK_EOF:   printf("eof at (%zu, %zu)", token->pos.line, token->pos.col); break;
    case TOK_ERROR: printf("error(%s: %s at %d) at (%zu, %zu)", token->error.str, token->error.c_file, token->error.c_line, token->pos.line, token->pos.col); break;
    case TOK_VAL:   printf("val(%.*s) at (%zu, %zu)", (int)token->val.len, token->val.cstr, token->pos.line, token->pos.col); break;
    case SEP_NEWLINE:   printf("sep(\\n) at (%zu, %zu)", token->pos.line, token->pos.col); break;
    case TOK_IDENT: printf("ident(%.*s) at (%zu, %zu)", (int)token->ident.len, token->ident.cstr, token->pos.line, token->pos.col); break;
    default: assert(false && "UNREACHABLE");
  }
}
void print_token_pretty(Token* token) {
  switch (token->kind) {
    #define X(a, b, c) case a: printf("op(%s)", b); break;
     OPERATORS(X)
    #undef X
    #define X(a, b, c) case a: printf("sep(%s)", b); break;
     SEPARATORS(X)
    #undef X
    #define X(a, b) case a: printf("keyword(%s)", b); break;
     KEYWORDS(X)
    #undef X
    case TOK_EOF:   printf("eof"); break;
    case TOK_ERROR: printf("error(%s: %s at %d)", token->error.str, token->error.c_file, token->error.c_line); break;
    case TOK_VAL:   printf("val(%.*s)", (int)token->val.len, token->val.cstr); break;
    case TOK_IDENT: printf("ident(%.*s)", (int)token->ident.len, token->ident.cstr); break;
    case SEP_NEWLINE: printf("sep(\\n)"); break;
    default: assert(false && "UNREACHABLE");
  }
}
