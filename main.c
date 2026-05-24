#include "parser.h"
#include <stdio.h>

int main(void) {
  char buf[4096] = {0};
  FILE *fd = fopen("./example.raon", "r");
  if (!fd) {
    perror("Failed to open file.");
    return 1;
  }

  fread(buf, 1, sizeof(buf) - 1, fd);
  if (ferror(fd)) {
    perror("Failed to read file.");
    return 2;
  }

  struct vector_of_raon_entry *entries = raon_parse(buf);
  printf("Ret: %p\n", entries);
}
