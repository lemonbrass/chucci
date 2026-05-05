#ifndef PP2_MACRO_H
#define PP2_MACRO_H

#include <compiler.h>
#include <da_intern.h>
#include <token_source.h>
#include <lexer.h>
#include <da_internmap.h>

struct Preprocessor2;


typedef internedmap_t(TokenArray) MacroCallArgMap;

typedef struct MacroDef{
  bool is_functionlike;
  kvec_t(interned_str) argnames;
  TokenArray body;
  interned_str name;
} MacroDef;

void free_macro_def(MacroDef* def);
void macro_def(struct Preprocessor2* pp2);
void macro_use(struct Preprocessor2* pp2, Token* token);

#endif
