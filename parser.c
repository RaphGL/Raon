#include "parser.h"
#include "lexer.h"
#include <stdbool.h>
#include <stdint.h>

#define VEC_IMPLEMENTATION
#define VEC_ITEM_TYPE struct raon_value
#define VEC_SUFFIX raon_value
#include "vector.h"

#define VEC_ITEM_TYPE struct raon_entry
#define VEC_SUFFIX raon_entry
#include "vector.h"

struct raon_entry raon_parse_entry(struct raon_lexer *lexer, struct raon_token first_token) {
   const struct raon_entry error_val = { .field_type = raon_field_type_error };

   struct raon_entry entry = { 0 };
   switch (first_token.type) {
   case raon_token_type_field:
   case raon_token_type_string:
      entry.field_type = raon_field_type_string;
      entry.str_field = first_token.str_val;
      break;
   case raon_token_type_int:
      entry.field_type = raon_field_type_int;
      entry.int_field = first_token.int_val;
      break;
   default:
      return error_val;
   }

   struct raon_token token = raon_lexer_eat(lexer);
   if (token.type != raon_token_type_equal) {
      return error_val;
   }

   token = raon_lexer_eat(lexer);
   struct raon_value val = raon_parse_value(lexer, token);
   if (val.type == raon_value_type_error) {
      return error_val;
   }
   entry.value = val;

   return entry;
}

struct raon_value raon_parse_value(struct raon_lexer *lexer, struct raon_token first_token) {
   const struct raon_value error_val = { .type = raon_value_type_error };

   struct raon_value val = { 0 };

   switch (first_token.type) {
   case raon_token_type_string:
      val.type = raon_value_type_string;
      val.str_val = first_token.str_val;
      break;
   case raon_token_type_bool:
      val.type = raon_value_type_bool;
      val.bool_val = first_token.bool_val;
      break;
   case raon_token_type_int:
      val.type = raon_value_type_int;
      val.int_val = first_token.int_val;
      break;
   case raon_token_type_block_open:
      val.type = raon_value_type_block;
      val.block_val = raon_parse_block(lexer, first_token);
      break;
   case raon_token_type_array_open:
      val.type = raon_value_type_array;
      val.array_val = raon_parse_array(lexer, first_token);
      break;
   default:
      return error_val;
   }

   struct raon_token token = raon_lexer_eat(lexer);
   if (token.type != raon_token_type_newline && token.type != raon_token_type_comma) {
      return error_val;
   }

   return val;
}

struct vector_of_raon_value *raon_parse_array(
    struct raon_lexer *lexer, struct raon_token first_token) {
   if (first_token.type != raon_token_type_array_open) {
     return NULL; 
   }

   struct vector_of_raon_value *values = vec_new_raon_value();

   for (;;) {
      struct raon_token token = raon_lexer_eat(lexer);
      if (token.type == raon_token_type_array_close) {
         break;
      }

      struct raon_value val = raon_parse_value(lexer, token);
      if (val.type == raon_value_type_error) {
         vec_free_raon_value(values);
        return NULL; 
      }

      vec_push_raon_value(values, val);
   }

   return values;
}

struct vector_of_raon_entry *raon_parse_block(
    struct raon_lexer *lexer, struct raon_token first_token) {
   if (first_token.type != raon_token_type_block_open) {
      return NULL;
   }

   struct vector_of_raon_entry *entries = vec_new_raon_entry();

   for (;;) {
      struct raon_token token = raon_lexer_eat(lexer);
      if (token.type == raon_token_type_error || token.type == raon_token_type_eof) {
         vec_free_raon_entry(entries);
         return NULL;
      }
      if (token.type == raon_token_type_block_close) {
         break;
      }
      if (token.type == raon_token_type_newline) {
        continue;
      }

      struct raon_entry entry = raon_parse_entry(lexer, token);
      if (entry.field_type == raon_field_type_error) {
         vec_free_raon_entry(entries);
         return NULL;
      }

      vec_push_raon_entry(entries, entry);
   }

   return entries;
}

// TODO: free tokens after they're used
struct vector_of_raon_entry *raon_parse(char *str) {
   struct raon_lexer lexer = raon_lexer_init(str);

   struct vector_of_raon_entry *entries = vec_new_raon_entry();
   for (;;) {
      struct raon_token token = raon_lexer_eat(&lexer);
      if (token.type == raon_token_type_error || token.type == raon_token_type_eof) {
         break;
      }
      if (token.type == raon_token_type_newline) {
        continue; 
      }

      struct raon_entry entry = raon_parse_entry(&lexer, token);
      if (entry.value.type != raon_value_type_error) {
         vec_push_raon_entry(entries, entry);
      }
   }

   return entries;
}
