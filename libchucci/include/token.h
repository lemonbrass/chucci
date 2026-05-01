#ifndef TOKEN_H
#define TOKEN_H

#include <da_string.h>
#include <cursor.h>
#include <da_intern.h>


typedef enum {
  TOK_EOF,
  TOK_ERROR,
  TOK_IDENT,
  TOK_NUM,
  TOK_OP,
  TOK_SEMI,
} TokenKind;

typedef struct {
  interned_str name;
} TokenIdent;

typedef struct {
  string_view num;
} TokenNum;

typedef struct {
  interned_str op;
} TokenOp;

typedef struct {
  CursorMark pos;
  TokenKind kind;
  union {
    TokenIdent ident;
    TokenNum num;
    TokenOp op;
  };
} Token;

#endif
