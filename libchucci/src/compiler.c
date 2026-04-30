#include <da_string.h>
#include <thirdparty/kvec.h>
#include <compiler.h>

CompilerOpt* new_opt() {
  CompilerOpt* opt = malloc(sizeof(CompilerOpt));
  kv_init(opt->include_dirs);
  return opt;
}

void opt_include_dir(CompilerOpt* opt, string dir) {
  kv_push(string, opt->include_dirs, dir);
}

void free_opt(CompilerOpt** opt) {
  for (size_t i = 0; i < kv_size((*opt)->include_dirs); i++) {
    free_str(&kv_A((*opt)->include_dirs, i));
  }
  kv_destroy((*opt)->include_dirs);
  free(*opt);
  *opt = NULL;
}
