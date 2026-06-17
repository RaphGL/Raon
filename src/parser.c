#include "raon.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

#define VEC_IMPLEMENTATION
#define VEC_ITEM_TYPE struct raon_value
#define VEC_SUFFIX raon_value
#include "../vendor/vector.h"

#define VEC_ITEM_TYPE struct raon_entry
#define VEC_SUFFIX raon_entry
#include "../vendor/vector.h"

// returns true on success
bool raon_is_valid_separator(
    struct raon_token first_token, const enum raon_token_type *optional_separator) {
   return first_token.type == raon_token_type_newline || first_token.type == raon_token_type_comma
       || (optional_separator != NULL && first_token.type == *optional_separator);
}

struct raon_entry raon_parse_entry(struct raon_lexer *lexer, struct raon_token first_token) {
   const struct raon_entry error_val = { .key_type = raon_key_type_error };

   struct raon_entry entry = { 0 };
   switch (first_token.type) {
   case raon_token_type_key:
   case raon_token_type_string:
      entry.key_type = raon_key_type_string;
      entry.str_key = first_token.str_val;
      break;

   case raon_token_type_int:
      entry.key_type = raon_key_type_int;
      entry.int_key = first_token.int_val;
      break;

   default:
      return error_val;
   }

   struct raon_token token = raon_lexer_eat(lexer);
   // if a <key>.<subkey> is found we just insert a single key into the block and move on
   // there's no need for type checking as a vec_len(block) == 1 is always valid
   if (token.type == raon_token_type_dot) {
      token = raon_lexer_eat(lexer);
      entry.value.type = raon_value_type_block;
      entry.value.block_val = vec_new_raon_entry();
      if (!entry.value.block_val) {
         return error_val;
      }
      vec_push_raon_entry(entry.value.block_val, raon_parse_entry(lexer, token));
      return entry;
   }
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

   case raon_token_type_float:
      val.type = raon_value_type_float;
      val.float_val = first_token.float_val;
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

      token = raon_lexer_eat(lexer);
      const enum raon_token_type array_close = raon_token_type_array_close;
      if (!raon_is_valid_separator(token, &array_close)) {
         vec_free_raon_value(values);
         return NULL;
      }
      if (token.type == raon_token_type_array_close) {
         break;
      }
   }

   // do type checking
   if (vec_len_raon_value(values) > 1) {
      struct raon_value value1, value2;
      vec_get_raon_value(values, 0, &value1);
      for (size_t i = 1; i < vec_len_raon_value(values); i++) {
         vec_get_raon_value(values, i, &value2);
         if (value1.type != value2.type) {
            vec_free_raon_value(values);
            return NULL;
         }
      }
   }

   return values;
}

void raon_free_values(struct vector_of_raon_value *values) {
   for (size_t i = 0; i < vec_len_raon_value(values); i++) {
      struct raon_value value;
      vec_get_raon_value(values, i, &value);
      switch (value.type) {
      case raon_value_type_array:
         raon_free_values(value.array_val);
         break;

      case raon_value_type_block:
         raon_free_entries(value.block_val);
         break;

      default:
         // do nothing
         break;
      }
   }
   vec_free_raon_value(values);
}

void raon_free_entries(struct vector_of_raon_entry *entries) {
   for (size_t i = 0; i < vec_len_raon_entry(entries); i++) {
      struct raon_entry entry;
      vec_get_raon_entry(entries, i, &entry);
      switch (entry.value.type) {
      case raon_value_type_block:
         raon_free_entries(entry.value.block_val);
         break;

      case raon_value_type_array:
         raon_free_values(entry.value.array_val);
         break;

      default:
         // do nothing
         break;
      }
   }
   vec_free_raon_entry(entries);
}

static bool raon_entry_types_match(struct vector_of_raon_entry *entries) {
   if (vec_len_raon_entry(entries) <= 1) {
      return true;
   }
   struct raon_entry entry1, entry2;
   vec_get_raon_entry(entries, 0, &entry1);
   for (size_t i = 1; i < vec_len_raon_entry(entries); i++) {
      vec_get_raon_entry(entries, i, &entry2);
      if (entry1.key_type != entry2.key_type) {
         return false;
      }
   }
   return true;
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
      if (entry.key_type == raon_key_type_error) {
         vec_free_raon_entry(entries);
         return NULL;
      }
      vec_push_raon_entry(entries, entry);

      token = raon_lexer_eat(lexer);
      const enum raon_token_type block_close = raon_token_type_block_close;
      if (!raon_is_valid_separator(token, &block_close)) {
         vec_free_raon_entry(entries);
         return NULL;
      }
      if (token.type == raon_token_type_block_close) {
         break;
      }
   }

   if (!raon_entry_types_match(entries)) {
      vec_free_raon_entry(entries);
      return NULL;
   }
   return entries;
}

struct vector_of_raon_entry *raon_parse(char *str, size_t len) {
   struct raon_lexer lexer = raon_lexer_init(str, len);

   struct vector_of_raon_entry *entries = vec_new_raon_entry();
   for (;;) {
      struct raon_token token = raon_lexer_eat(&lexer);
      if (token.type == raon_token_type_eof) {
         break;
      }
      if (token.type == raon_token_type_newline) {
         continue;
      }
      if (token.type == raon_token_type_error) {
         return NULL;
      }

      struct raon_entry entry = raon_parse_entry(&lexer, token);
      if (entry.value.type != raon_value_type_error) {
         vec_push_raon_entry(entries, entry);
      }

      token = raon_lexer_eat(&lexer);

      if (!raon_is_valid_separator(token, NULL)) {
         break;
      }
   }

   if (!raon_entry_types_match(entries)) {
      raon_free_entries(entries);
      return NULL;
   }

   return entries;
}

static void raon_print_indentation(struct raon_print_ctx ctx) {
   char *indent = ctx.indent ? ctx.indent : "   ";
   for (size_t i = 0; i < ctx.indent_level; i++) {
      printf("%s", indent);
   }
}

void raon_print_value(struct raon_print_ctx ctx, struct raon_value value) {
   switch (value.type) {
   case raon_value_type_bool:
      printf("%s", value.bool_val ? "true" : "false");
      break;

   case raon_value_type_int:
      printf("%ld", value.int_val);
      break;

   case raon_value_type_float:
      printf("%lf", value.float_val);
      break;

   case raon_value_type_string: {
      struct raon_str_slice str = value.str_val;
      printf("\"%.*s\"", (int)str.len, str.ptr);
   } break;

   case raon_value_type_array:
      raon_print_array(ctx, value.array_val);
      break;

   case raon_value_type_block:
      printf("{\n");
      ++ctx.indent_level;
      raon_print_entries(ctx, value.block_val);
      --ctx.indent_level;
      raon_print_indentation(ctx);
      printf("}");
      break;

   case raon_value_type_error:
      printf("(unsupported type)");
      break;
   }
}

void raon_print_array(struct raon_print_ctx ctx, struct vector_of_raon_value *array) {
   if (!array) {
      printf("[]");
      return;
   }

   printf("[");
   for (size_t i = 0; i < vec_len_raon_value(array); i++) {
      struct raon_value val;
      vec_get_raon_value(array, i, &val);
      raon_print_value(ctx, val);
      if (i < vec_len_raon_value(array) - 1) {
         printf(", ");
      }
   }
   printf("]");
}

void raon_print_entry(struct raon_print_ctx ctx, struct raon_entry entry) {
   raon_print_indentation(ctx);
   switch (entry.key_type) {
   case raon_key_type_string: {
      struct raon_str_slice str = entry.str_key;
      bool contains_whitespace = false;
      for (size_t i = 0; i < entry.str_key.len; i++) {
         if (isspace(str.ptr[i])) {
            contains_whitespace = true;
         }
      }

      if (contains_whitespace) {
         printf("\"%.*s\" = ", (int)str.len, str.ptr);
      } else {
         printf("%.*s = ", (int)str.len, str.ptr);
      }
   } break;

   case raon_key_type_int:
      printf("%lu = ", entry.int_key);
      break;

   case raon_key_type_error:
      printf("(unsupported type) = ");
   }

   raon_print_value(ctx, entry.value);
   printf("\n");
}

void raon_print_entries(struct raon_print_ctx ctx, struct vector_of_raon_entry *entries) {
   if (!entries) {
      printf("(null)");
      return;
   }
   for (size_t i = 0; i < vec_len_raon_entry(entries); i++) {
      struct raon_entry entry;
      vec_get_raon_entry(entries, i, &entry);
      raon_print_entry(ctx, entry);
   }
}
