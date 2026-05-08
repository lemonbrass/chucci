#ifndef COMPILER_H
#define COMPILER_H

#include <memscope.h>
#include <da_internmap.h>
#include <da_string.h>
#include <setjmp.h>
#include <token.h>
#include <da_intern.h>
#include <da_path.h>
#include <da_string.h>
#include <stddef.h>
#include <thirdparty/kvec.h>


struct MacroDef;

typedef struct {
  kvec_t(string) include_dirs;
} CompilerOpt;

typedef struct {
  CompilerOpt* options;
  kvec_t(Path) included_files;
  kvec_t(interned_str) macro_stack;
  kvec_t(MemScope*) memstack;
  InternTable* table;
  kvec_t(string) source_stack;
  interned_str keywords[__token_kind_count];
  interned_str preprocessor_cmds[__preprocessor_cmd_len];
  da_string buf; // for internal memory reuse
  TokenArray token_buf;
  internedmap_t(struct MacroDef) macros;
  jmp_buf* onerror;
} ChucciCompiler;

ChucciCompiler new_compiler(CompilerOpt* opt, string source, jmp_buf* onerror);
TokenArray compiler_compile(ChucciCompiler* compiler);
void add_source(ChucciCompiler* ctx, string source);
CompilerOpt* new_opt();
void opt_include_dir(CompilerOpt* opt, string dir);
void free_opt(CompilerOpt** opt);

void push_memscope(ChucciCompiler* ctx, MemScope* scope);
void pop_memscope(ChucciCompiler* ctx);

void free_compiler(ChucciCompiler* ctx);
void initiate_error(ChucciCompiler* ctx);


#endif
