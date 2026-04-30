#include <setjmp.h>
#include <stdio.h>

#define RUN_TEST(func) do {\
  printf("[TEST] Running %s\n", #func);\
  if (setjmp(errbuf) == 0) {\
    func(errbuf);\
    printf("[TEST] %s passed\n", #func);\
  }\
  else printf("[TEST] %s failed\n", #func);\
} while(0)

void cursor1(jmp_buf buf);
void preprocess1_1(jmp_buf buf);
void preprocess1_2(jmp_buf buf);

jmp_buf errbuf;

jmp_buf assert_env;

int main() {
  RUN_TEST(cursor1);
  RUN_TEST(preprocess1_1);
  RUN_TEST(preprocess1_2);
  return 0;
}
