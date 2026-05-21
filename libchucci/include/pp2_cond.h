#ifndef PP2_COND_H
#define PP2_COND_H

#include <da_intern.h>
#include <token.h>
#include <preprocess_2.h>

#define PP_INFIX_OPERATORS(X) \
  X(OP_OR, ||, 30) \
  X(OP_AND, &&, 40) \
  X(OP_BIT_OR, |, 50) \
  X(OP_BIT_XOR, ^, 60) \
  X(OP_BIT_AND, &, 70) \
  X(OP_EQ, ==, 80) \
  X(OP_NEQ, !=, 80) \
  X(OP_LT, <, 90) \
  X(OP_GT, >, 90) \
  X(OP_LE, <=, 90) \
  X(OP_GE, >=, 90) \
  X(OP_SHL, <<, 100) \
  X(OP_SHR, >>, 100) \
  X(OP_ADD, +, 110) \
  X(OP_SUB, -, 110) \
  X(OP_MUL, *, 120) \
  X(OP_DIV, /, 120) \
  X(OP_MOD, %, 120) \

#define PP_PREFIX_OPERATORS(X) \
  X(OP_NOT, !, 130) \
  X(OP_BIT_NOT, ~, 130) \
  X(OP_ADD, +, 130) \
  X(OP_SUB, -, 130)

#define PP_PAREN_OPERATOR(X) \
  X(SEP_LPAREN, "(", 140)
  
#define PP_TERNARY_OPERATOR(X) \
  X(OP_QUESTION, "?", 25)


typedef struct Expr Expr;
typedef TokenKind OpKind;

// typedef enum ExprKind {
//   EK_BINARY,
//   EK_UNARY,
//   EK_TERNARY,
//   EK_DEFINED,
// } ExprKind;

// struct Expr {
//   ExprKind kind;
//   union {
//     struct {
//       Expr* left;
//       Expr* right;
//       OpKind op;
//     } binary;
//     struct {
//       OpKind op;
//       Expr* operand;
//     } unary;
//     struct {
//       Expr* cond;
//       Expr* if_true;
//       Expr* if_false;
//     } ternary;
//     struct {
//       interned_str name;
//     } defined;
//   };
// };

// conditional used
void cond_use(Preprocessor2* pp2);
#endif

