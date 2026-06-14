#include "src/parser.h"
#include <stdio.h>

#define BUF_SIZE 20 * 1024 * 1024

int main(void) {
   char *buf = malloc(BUF_SIZE);
   if (!buf) {
      perror("Failed to preallocate buffer");
      goto cleanup;
   }

   FILE *fd = fopen("./example.raon", "r");
   if (!fd) {
      perror("Failed to open file.");
      goto cleanup;
   }

   fread(buf, 1, BUF_SIZE, fd);
   if (ferror(fd)) {
      perror("Failed to read file.");
      fclose(fd);
      goto cleanup;
   }

   struct vector_of_raon_entry *entries = raon_parse(buf);
   printf("Ret: %p\n", (void *)entries);

   struct raon_print_ctx ctx = { 0 };
   raon_print_entries(ctx, entries);

cleanup:
   raon_free_entries(entries);
   fclose(fd);
   free(buf);
   return 0;
}
