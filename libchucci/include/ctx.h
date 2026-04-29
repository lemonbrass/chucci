#ifndef CTX_H
#define CTX_H

#include <da_path.h>
#include <da_string.h>
#include <stddef.h>
#include <thirdparty/kvec.h>


typedef struct {
  kvec_t(string) include_dirs;
  kvec_t(Path) included_files;
  da_string buf; // for internal memory reuse
} CompilerCtx;

CompilerCtx new_ctx();
void free_ctx(CompilerCtx* ctx);

#endif
