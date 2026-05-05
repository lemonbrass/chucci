#include <da_string.h>
#include <stdio.h>

int main() {
  da_string input = new_ds();
  while (true) {
    int ch;
    while ((ch = getchar()) != EOF && ch != '\n') {
      push_char_ds(&input, (unsigned char)ch);
    }
    
  }
  return 0;
}
