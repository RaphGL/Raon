#include "src/raon.h"
#include "vendor/ht.h"
#include <assert.h>
#include <stdio.h>

#define HT_IMPLEMENTATION
#include "vendor/ht.h"

#define BUF_SIZE 20 * 1024 * 1024

struct unit_test {
   char *type;
   char *input;
   bool success;
};

#define run_lexer_test(lexer_func)                                                                 \
   for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++) {                               \
      struct raon_lexer lex = raon_lexer_init(inputs[i].input, strlen(inputs[i].input));           \
      struct raon_token token = lexer_func(&lex);                                                  \
      printf("Testing %s: ", inputs[i].type);                                                      \
      assert((token.type != raon_token_type_error) == inputs[i].success);                          \
      printf("OK\n");                                                                              \
   }

void test_num_values() {
   struct unit_test inputs[] = { { "binary int", "0b110", true },
      { "hexadecimal int", "0xdeadbeef", true }, { "octal int", "0o72", true },
      { "decimal int", "999", true }, { "int separator", "10_000", true },
      { "int trailing separator", "10_000_", false },
      { "int starting separator", "_10_000", false }, { "positive float", "0.5", true },
      { "negative float", "-0.5", true }, { "double float", "0.5.1", false },
      { "negative double float", "-0.5.1", false }, { "float separator", "5_430.1_2", true } };

   run_lexer_test(raon_lexer_lex_num);
}

void test_string_values(void) {
   struct unit_test inputs[] = { { "string literal", "\"whatever string\"", true },
      { "multiline string", "\"whatever\nstring\nwith newlines\n\"", true },
      { "string without quotes", "missing quotes here", false },
      { "string without trailing quote", "\"missing one quote", false } };

   run_lexer_test(raon_lexer_lex_string);
}

void test_ident_values(void) {
   struct unit_test inputs[] = {
      { "false boolean", "false", true },
      { "true boolean", "true", true },
      { "snake case identifier", "some_identifier_key", true },
      { "snake case identifier 2", "_some_identifier_key", true },
      { "lisp case identifier", "some-identifier-key", true },
      { "wrong lisp case", "-some-identifier-key", false },
   };

   run_lexer_test(raon_lexer_lex_ident);
}

void test_entries(void) {
   // TODO
   struct unit_test inputs[] = {0};
}

void test_array(void) {
   // TODO
   struct unit_test inputs[] = {0};
}

void test_blocks(void) {
   // TODO
   struct unit_test inputs[] = {0};
}

int main(void) {
   test_num_values();
   test_string_values();
   test_ident_values();

   char *buf = malloc(BUF_SIZE);
   if (!buf) {
      perror("Failed to preallocate buffer");
      goto cleanup;
   }
   memset(buf, 0, BUF_SIZE);

   FILE *fd = fopen("./example.raon", "r");
   if (!fd) {
      perror("Failed to open file.");
      goto cleanup;
   }

   fread(buf, 1, BUF_SIZE - 1, fd);
   if (ferror(fd)) {
      perror("Failed to read file.");
      fclose(fd);
      goto cleanup;
   }

   struct vector_of_raon_entry *entries = raon_parse(VEC_DEFAULT_ALLOCATOR, buf, strlen(buf));
   printf("\n\n");
   printf("Ret: %p\n\n", (void *)entries);

   struct raon_print_ctx ctx = { 0 };
   raon_print_entries(ctx, entries);
   puts("\nAll tests passed.");

cleanup:
   raon_free_entries(entries);
   fclose(fd);
   free(buf);
   return 0;
}
