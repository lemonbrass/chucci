#ifndef PREPROCESS_2_H
#define PREPROCESS_2_H

#include <lexer.h>
#include <da_internmap.h>
#include <ctx.h>
#include <pp2_macro.h>

typedef enum {
  PT_MACRO,
  PT_CONDITIONAL,
} PP2TaskKind;

typedef kvec_t(Token) TokenStream;

typedef struct {
  PP2TaskKind kind;
  union {
    MacroTask macro;
    struct {} conditional;
  };
} PP2Task;

typedef struct Preprocessor2 {
  CompilerCtx* ctx;
  TokenStream stream;
  kvec_t(PP2Task) task_stack;
  internedmap_t(MacroDef) macros;
} Preprocessor2;

Preprocessor2 new_pp2(CompilerCtx* ctx);
TokenStream resolve_pp2(Preprocessor2* pp2, Lexer* lexer);

#endif
