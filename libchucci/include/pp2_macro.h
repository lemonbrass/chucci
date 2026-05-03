#ifndef PP2_MACRO_H
#define PP2_MACRO_H

#include "da_intern.h"
#include <lexer.h>
#include <da_internmap.h>
#include <ctx.h>

struct Preprocessor2;


typedef kvec_t(Token) MacroArg;

typedef struct {
  internedmap_t(MacroArg) args;
  kvec_t(Token) body;
  interned_str name;
} MacroTask;

typedef struct {
  kvec_t(interned_str) args;
  kvec_t(Token) body;
  interned_str name;
} MacroDef;

void macro_def(struct Preprocessor2* pp2, Lexer* lexer);

#endif
