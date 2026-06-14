#ifndef __PP_MACRO_H
#define __PP_MACRO_H

#include "compiler.h"
#include "frontend/cursor.h"
#include "frontend/token.h"
#include "frontend/token_stream.h"
#include "utils/linear_map.h"
#include "utils/string_interner.h"
#include "utils/vmem_arena.h"

typedef struct Preprocessor Preprocessor;

typedef struct MacroDef {
  Token name;
  TokenVec body;
  StringIDVec args;
  bool is_fnlike;
  Span span;
} MacroDef;

// links arg_name to passed tokens
LINEAR_MAP_DEF(StringID, TokenVec, MacroArgMap, macroargmap, VMEM_ARENA_ALLOC_INT)
LINEAR_MAP_DEF(StringID, MacroDef, MacroDefMap, macrodefmap, VMEM_ARENA_ALLOC_INT)

typedef struct MacroUseStream {
  MacroDef *def;
  VMEMArena *arena;
  MacroArgMap argmap;
  TokenStreamStack streams;
  bool _is_arg;
} MacroUseStream;


Token mu_next_token(TokenStream *ts, CompilerCtx *ctx);
Token mu_peek_token(TokenStream *ts, CompilerCtx *ctx);
void macro_def(Preprocessor *pp, CompilerCtx *ctx, Token token);
TokenStream macro_use(Preprocessor *pp, CompilerCtx *ctx, Token name, MacroDef *def);





#endif
