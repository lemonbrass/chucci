#ifndef TOKEN_H
#define TOKEN_H

#include <assert.h>
#include <da_string.h>
#include <cursor.h>
#include <da_intern.h>

extern const char* keyword_to_str[];
extern const char* op_to_str[];
extern const char* sep_to_str[];
extern const char* tok_to_str[];
extern bool is_op_table[256];

#define is_op(ch) (is_op_table[ch])

#define OPERATORS(X) \
    X(OP_SHL_EQ, "<<=", '<') \
    X(OP_SHR_EQ, ">>=", '>') \
    X(OP_ELLIPSIS, "...", '.') \
    X(OP_INC, "++", '+') \
    X(OP_DEC, "--", '-') \
    X(OP_EQ, "==", '=') \
    X(OP_NEQ, "!=", '!') \
    X(OP_LE, "<=", '<') \
    X(OP_GE, ">=", '>') \
    X(OP_AND, "&&", '&') \
    X(OP_OR, "||", '|') \
    X(OP_SHL, "<<", '<') \
    X(OP_SHR, ">>", '>') \
    X(OP_ADD_EQ, "+=", '+') \
    X(OP_SUB_EQ, "-=", '-') \
    X(OP_MUL_EQ, "*=", '*') \
    X(OP_DIV_EQ, "/=", '/') \
    X(OP_MOD_EQ, "%=", '%') \
    X(OP_AND_EQ, "&=", '&') \
    X(OP_OR_EQ, "|=", '|') \
    X(OP_XOR_EQ, "^=", '^') \
    X(OP_ARROW, "->", '-') \
    X(OP_ADD, "+", '+') \
    X(OP_SUB, "-", '-') \
    X(OP_MUL, "*", '*') \
    X(OP_DIV, "/", '/') \
    X(OP_MOD, "%", '%') \
    X(OP_ASSIGN, "=", '=') \
    X(OP_LT, "<", '<') \
    X(OP_GT, ">", '>') \
    X(OP_NOT, "!", '!') \
    X(OP_BIT_AND, "&", '&') \
    X(OP_BIT_OR, "|", '|') \
    X(OP_BIT_XOR, "^", '^') \
    X(OP_BIT_NOT, "~", '~') \
    X(OP_QUESTION, "?", '?') \
    X(OP_COLON, ":", ':') \
    X(OP_DOT, ".", '.') \
    X(OP_PREPROCESS, "#", '#')

#define SEPARATORS(X) \
  X(SEP_LPAREN, "(", '(') \
  X(SEP_RPAREN, ")", ')') \
  X(SEP_LCURLY, "{", '{') \
  X(SEP_RCURLY, "}", '}') \
  X(SEP_LSQ, "[", '[') \
  X(SEP_RSQ, "]", ']') \
  X(SEP_COMMA, ",", ',') \
  X(SEP_SEMI, ";", ';') \
  X(SEP_COLON, ":", ':') \

#define KEYWORDS(X) \
  X(KW_IF, "if") \
  X(KW_ELSE, "else") \
  X(KW_WHILE, "while") \
  X(KW_FOR, "for") \
  X(KW_RETURN, "return") \
  X(KW_BREAK, "break") \
  X(KW_CONTINUE, "continue") \
  X(KW_STRUCT, "struct") \
  X(KW_ENUM, "enum") \
  X(KW_TYPEDEF, "typedef") \
  X(KW_CONST, "const") \
  X(KW_STATIC, "static") \
  X(KW_VOID, "void") \
  X(KW_INT, "int") \
  X(KW_FLOAT, "float") \
  X(KW_CHAR, "char") \
  X(KW_SIZEOF, "sizeof")

#define PREPROCESSOR_CMD(X) \
X(PP_DEFINE, "define") \
X(PP_UNDEF, "undef") \
X(PP_IF, "if") \
X(PP_IFDEF, "ifdef") \
X(PP_IFNDEF, "ifndef") \
X(PP_ELIF, "elif") \
X(PP_ELSE, "else") \
X(PP_ENDIF, "endif") \
X(PP_ERROR, "error") \
X(PP_LINE, "line") \
X(PP_PRAGMA, "pragma") \

typedef enum {
  #define X(a, b) a,
  PREPROCESSOR_CMD(X)
  #undef X
  __preprocessor_cmd_len
} PPCmd;


typedef enum TokenKind {
  #define X(a, b, c) a,
  OPERATORS(X)
  #undef X
  #define X(a, b) a,
  KEYWORDS(X)
  #undef X
  #define X(a, b, c) a,
  SEPARATORS(X)
  #undef X
  TOK_EOF,
  TOK_ERROR,
  TOK_IDENT,
  TOK_VAL,
  SEP_NEWLINE,
  __token_kind_count,
} TokenKind;

typedef struct {
  const char* str;
  const char* c_file;
  int c_line;
} TokenError;

typedef struct {
  CursorMark pos;
  TokenKind kind;
  union {
    interned_str ident;
    string_view val;
    TokenError error;
  };
} Token;

#define new_token(pos, data) \
   _Generic(data,\
   interned_str: new_tok_ident, \
   string_view: new_tok_val, \
   int: new_tok_simple)(pos, data)



#define ERROR_TOKEN(pos, error) new_tok_error(pos, __LINE__, __FILE__, error)
#define EOF_TOKEN(p) ((Token) { .kind=TOK_EOF, .pos=p })


Token new_tok_error(CursorMark pos, int c_line, const char* c_file, const char* error);
Token new_tok_ident(CursorMark pos, interned_str name);
Token new_tok_ident(CursorMark pos, interned_str name);
Token new_tok_val(CursorMark pos, string_view val);
Token new_tok_simple(CursorMark pos, TokenKind keyword);

void print_token(Token* token);
void print_token_pretty(Token* token);

#endif
