#include "lexer.h"
#include <stdio.h>

int main(void) {
   FILE *raon_fd = fopen("./example.raon", "r");
   if (!raon_fd) {
      perror("Failed to open raon file");
      return 1;
   }

   char buf[4096] = { 0 };
   if (fread(buf, 1, sizeof(buf), raon_fd) == 0 && !feof(raon_fd)) {
      perror("Failed to read raon file");
      return 2;
   }
   fclose(raon_fd);

   struct raon_lexer lex = raon_lexer_init(buf);
   return 0;
}
