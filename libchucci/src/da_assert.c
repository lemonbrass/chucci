#include <stdio.h>
#include <stdlib.h>

#ifdef ASSERT_MODE_TEST
  #include <setjmp.h>
  extern jmp_buf assert_env;
#endif

void __handle_assert(const char *cond, const char *file, int line) {
  printf("Assertion assert(%s) failed in file %s at line %d\n", cond, file, line);
  #ifdef ASSERT_MODE_TEST
  longjmp(assert_env, 1);
  #else
  exit(EXIT_FAILURE);
  #endif
}

