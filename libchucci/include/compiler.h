#ifndef __COMPILER_H
#define __COMPILER_H

#include "utils/string.h"
#include "utils/smallvec.h"
#include "utils/vmem_arena.h"

SMALLVEC_DEF(String, StringStack, source_stack);

typedef struct {
  StringStack sources;
  StringStack included_dirs;
  VMEMArena arena;
} CompilerCtx;

CompilerCtx compiler_ctx_new();
void cc_compile(CompilerCtx* ctx);

#endif
