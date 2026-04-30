#ifndef COMPILER_H
#define COMPILER_H


#include <da_string.h>

typedef struct {
  kvec_t(string) include_dirs;
} CompilerOpt;

typedef struct {
  CompilerOpt* options;
} ChucciCompiler;

CompilerOpt* new_opt();
void opt_include_dir(CompilerOpt* opt, string dir);
void free_opt(CompilerOpt** opt);

#endif
