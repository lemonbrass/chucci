#ifndef COMPILER_H
#define COMPILER_H

#include <utils/bufferpool.h>
#include <utils/da_arena.h>
#include <utils/da_internmap.h>
#include <utils/da_string.h>
#include <setjmp.h>
#include <frontend/token.h>
#include <utils/da_intern.h>
#include <utils/da_path.h>
#include <utils/da_string.h>
#include <stddef.h>
#include <thirdparty/kvec.h>


typedef struct MacroDef MacroDef;
typedef struct ChucciCompiler ChucciCompiler;

typedef struct CompilerOpt {
  kvec_t(string) include_dirs;
  kvec_t(string) source_stack;
} CompilerOpt;

  
extern interned_str keywords[__token_kind_count];
extern interned_str preprocessor_cmds[__preprocessor_cmd_len];

// for PP1, PP2 and LEXER
typedef struct PreprocessorCtx {
  internedmap_t(struct MacroDef) macros;
  kvec_t(Path) included_files;
  kvec_t(interned_str) macro_stack;
  MacroDef* bufdef;

  ChucciCompiler* global;
} PPCtx;

typedef struct ChucciCompiler {
  bufferpool_t(da_string) strings;
  bufferpool_t(TokenArray) tokenarrays;

  CompilerOpt* options;
  PPCtx ppctx;
  InternTable* table;
  arena_t* arena;
  jmp_buf* onerror;
} ChucciCompiler;

ChucciCompiler* new_compiler(CompilerOpt* opt, jmp_buf* onerror);
TokenArray compiler_preprocess(ChucciCompiler* ctx);
void opt_add_source(CompilerOpt* opt, string source);
CompilerOpt* new_opt();
void opt_include_dir(CompilerOpt* opt, string dir);
void free_opt(CompilerOpt** opt);

void free_compiler(ChucciCompiler** ctx);
_Noreturn void initiate_error(ChucciCompiler* ctx);


#endif
