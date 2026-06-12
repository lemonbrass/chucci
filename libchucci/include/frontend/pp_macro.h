#ifndef __PP_MACRO_H
#define __PP_MACRO_H

#include "compiler.h"
#include "frontend/token.h"
#include "frontend/token_stream.h"
#include "utils/linear_map.h"
#include "utils/string_interner.h"

typedef struct Preprocessor Preprocessor;

typedef struct MacroDef {
  TokenVec body;
  StringIDVec args;
  bool is_fnlike;
} MacroDef;

LINEAR_MAP_DEF(StringID, MacroDef, MacroDefMap, macrodefmap, VMEM_ARENA_ALLOC_INT)

void macro_def(Preprocessor *pp, CompilerCtx *ctx, Token token);





#endif
