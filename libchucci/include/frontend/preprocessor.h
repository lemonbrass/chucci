#ifndef __PREPROCESSOR_H
#define __PREPROCESSOR_H

#include "compiler.h"
#include "frontend/pp_macro.h"
#include "frontend/token_stream.h"

// typedef struct MacroDef MacroDef;


struct Preprocessor {
  MacroDefMap macros;
  TokenStreamStack streams;
};

// Even though CompilerCtx has a Lexer, I added the Lexer* argument
// because it explicitly conveys that the Lexer should be initialized
Preprocessor *pp_new(CompilerCtx *ctx, Lexer *lexer);
Token pp_next_token(Preprocessor *pp, CompilerCtx *ctx);



#endif
