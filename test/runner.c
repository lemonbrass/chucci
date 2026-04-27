#include <stdio.h>

#define RUN_TEST(func) do {\
  printf("Running %s\n", #func);\
  if (func()) printf("[TEST] %s passed\n", #func); else printf("[TEST] %s failed\n", #func);\
} while(0)


int main() {
  return 0;
}
