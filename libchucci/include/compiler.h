#ifndef COMPILER_H
#define COMPILER_H

#include <da_string.h>
#include <setjmp.h>
#include <token.h>
#include <da_intern.h>
#include <da_path.h>
#include <da_string.h>
#include <stddef.h>
#include <thirdparty/kvec.h>


typedef struct {
  kvec_t(string) include_dirs;
} CompilerOpt;

typedef struct {
  CompilerOpt* options;
  kvec_t(Path) included_files;
  InternTable* table;
  kvec_t(string) source_stack;
  interned_str keywords[__token_kind_count];
  interned_str preprocessor_cmds[__preprocessor_cmd_len];
  da_string buf; // for internal memory reuse
  jmp_buf* onerror;
} ChucciCompiler;

ChucciCompiler new_compiler(CompilerOpt* opt, string source, jmp_buf* onerror);
TokenArray compiler_preprocess(ChucciCompiler* ctx);
void add_source(ChucciCompiler* ctx, string source);
CompilerOpt* new_opt();
void opt_include_dir(CompilerOpt* opt, string dir);
void free_opt(CompilerOpt** opt);

void free_compiler(ChucciCompiler* ctx);
void initiate_error(ChucciCompiler* ctx);


#endif
