#ifndef PREPROCESS_2_H
#define PREPROCESS_2_H

#include <thirdparty/kvec.h>
#include <token_source.h>
#include <da_internmap.h>
#include <ctx.h>
#include <pp2_macro.h>

typedef struct Preprocessor2 {
  CompilerCtx* ctx;
  TokenArray stream;
  internedmap_t(MacroDef) macros;
  TokenSource* token_source;
} Preprocessor2;

void free_pp2(Preprocessor2* pp2);
Preprocessor2 new_pp2(CompilerCtx* ctx, TokenSource* token_source);
TokenArray resolve_pp2(Preprocessor2* pp2);

#endif
