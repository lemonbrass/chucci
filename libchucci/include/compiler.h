#ifndef __COMPILER_H
#define __COMPILER_H

#include "utils/file.h"
#include "utils/diagnostics.h"
#include "utils/string.h"
#include "utils/smallvec.h"
#include "utils/string_interner.h"
#include "utils/vmem_arena.h"
#include <setjmp.h>

SMALLVEC_DEF(String, StringVec, stringvec);

typedef struct Lexer Lexer;
typedef struct Preprocessor Preprocessor;

typedef struct {
  FileVec sources;
  StringVec included_dirs;
  VMEMArena *arena;
  StringInterner *interner;
  DiagnosticEngine *engine;
  Lexer* lexer;
  Preprocessor* preprocessor;
  jmp_buf *onerror;
} CompilerCtx;

CompilerCtx *compiler_new(jmp_buf *onerror);
void cc_add_source(CompilerCtx *ctx, File source);
void cc_compile(CompilerCtx* ctx);
void cc_preamble(CompilerCtx* ctx);
void cc_free(CompilerCtx *ctx);

#endif
