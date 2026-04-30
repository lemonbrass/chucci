#ifndef CTX_H
#define CTX_H

#include <da_path.h>
#include <da_string.h>
#include <stddef.h>
#include <thirdparty/kvec.h>
#include <compiler.h>

typedef struct {
  CompilerOpt* options;
  kvec_t(Path) included_files;
  da_string buf; // for internal memory reuse
} CompilerCtx;

CompilerCtx new_ctx(CompilerOpt* options);
void free_ctx(CompilerCtx* ctx);

#endif
