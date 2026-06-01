#ifndef PREPROCESS_2_H
#define PREPROCESS_2_H

#include <thirdparty/kvec.h>
#include <frontend/token_source.h>
#include <utils/da_internmap.h>
#include <compiler.h>
#include <frontend/pp2_macro.h>

typedef struct Preprocessor2 {
  PPCtx* ctx;
  TokenArray stream;
  TokenSource* token_source;
} Preprocessor2;

Preprocessor2 new_pp2(PPCtx* ctx, TokenSource* token_source);
TokenArray resolve_pp2(Preprocessor2* pp2);
TokenArray recursively_expand(Preprocessor2* pp2, TokenSource* token_source);

#endif
