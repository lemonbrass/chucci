#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
  TOK_EOF,
  TOK_ERROR,
  TOK_IDENT,
  TOK_NUM,
  TOK_OP,
  TOK_SEMI,
} TokenKind;

typedef struct {
  
} TokenIdent;

#endif
