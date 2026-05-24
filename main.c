#include "parser.h"
#include <stdio.h>

int main(void) {
   char buf[4096] = { 0 };
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

   for (size_t i = 0; i < vec_len_raon_entry(entries); i++) {
      struct raon_entry entry;
      vec_get_raon_entry(entries, i, &entry);

      if (entry.value.type == raon_value_type_array) {
         switch (entry.field_type) {
         case raon_field_type_string:
            printf("%s: ", entry.str_field);
            break;
         case raon_field_type_int:
            printf("%ld: ", entry.int_field);
            break;
         }

         printf("[ ");
         for (size_t j = 0; j < vec_len_raon_value(entry.value.array_val); j++) {
            struct raon_value val;
            vec_get_raon_value(entry.value.array_val, j, &val);
            switch (val.type) {
            case raon_value_type_int:
               printf("%ld ", val.int_val);
               break;
            case raon_value_type_bool:
               printf(val.bool_val ? "true" : "false");
               break;
            case raon_value_type_string:
               printf("%s", val.str_val);
               break;
            }
         }
         printf("]\n");
      }

      if (entry.value.type == raon_value_type_block) {
         switch (entry.field_type) {
         case raon_field_type_string:
            printf("%s: ", entry.str_field);
            break;
         case raon_field_type_int:
            printf("%ld: ", entry.int_field);
            break;
         }
         printf("{ ");
         for (size_t j = 0; j < vec_len_raon_entry(entry.value.block_val); j++) {
            struct raon_entry val;
            vec_get_raon_entry(entry.value.block_val, j, &val);
            switch (val.field_type) {
            case raon_field_type_int:
               printf("%ld: ", val.int_field);
               break;
            case raon_field_type_string:
               printf("%s: ", val.str_field);
               break;
            }
            switch (val.value.type) {
            case raon_value_type_int:
               printf("%ld ", val.value.int_val);
               break;
            case raon_value_type_bool:
               printf(val.value.bool_val ? "true" : "false");
               break;
            case raon_value_type_string:
               printf("%s", val.value.str_val);
               break;
            }
            printf("\n");
         }
         printf("}\n");
      }
   }
}
