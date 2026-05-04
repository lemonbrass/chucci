#ifndef CTX_H
#define CTX_H

#include <setjmp.h>
#include <token.h>
#include <da_intern.h>
#include <da_path.h>
#include <da_string.h>
#include <stddef.h>
#include <thirdparty/kvec.h>
#include <compiler.h>

typedef struct {
  CompilerOpt* options;
  kvec_t(Path) included_files;
  InternTable* table;
  kvec_t(string) source_stack;
  interned_str keywords[__token_kind_count];
  interned_str preprocessor_cmds[__preprocessor_cmd_len];
  da_string buf; // for internal memory reuse
  jmp_buf* onerror;
} CompilerCtx;

CompilerCtx new_ctx(CompilerOpt* options, string source, jmp_buf* onerror);
void free_ctx(CompilerCtx* ctx);
void initiate_error(CompilerCtx* ctx, const char* msg);

#endif
