#ifndef __COMPILER_H
#define __COMPILER_H

#include "utils/string.h"
#include "utils/vec.h"

VEC_DEF(String, StringStack, source_stack);

typedef struct {
  StringStack sources;
  StringStack included_dirs;
} CompilerCtx;

#endif
