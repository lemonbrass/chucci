#ifndef PP2_MACRO_H
#define PP2_MACRO_H

#include <frontend/token.h>
#include <compiler.h>
#include <utils/da_intern.h>
#include <frontend/token_source.h>
#include <frontend/lexer.h>
#include <utils/da_internmap.h>

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
void macro_use(struct Preprocessor2* pp2, Token* token, TokenArray* out);
void print_macro_def(MacroDef* def);

#endif
