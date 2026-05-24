#include "lexer.h"
#include <stdbool.h>
#include <stdint.h>

enum raon_value_type {
   raon_value_type_string,
   raon_value_type_int,
   raon_value_type_bool,
   raon_value_type_block,
   raon_value_type_array,
   raon_value_type_error,
};

struct raon_value {
   enum raon_value_type type;
   union {
      char *str_val;
      intptr_t int_val;
      bool bool_val;
      struct raon_entry_vec *block_val;
      struct raon_value_vec *array_val;
   };
};

enum raon_field_type {
   raon_field_type_string,
   raon_field_type_int,
   raon_field_type_error,
};

struct raon_entry {
   enum raon_field_type field_type;
   union {
      char *str_field;
      intptr_t int_field;
   };
   struct raon_value value;
};

struct raon_parser_array {
   enum raon_value_type type;
   struct raon_value_vec *values;
};

#define VEC_ITEM_TYPE struct raon_value
#define VEC_SUFFIX raon_value
#define VEC_IMPLEMENTATION
#include "vector.h"

#define VEC_ITEM_TYPE struct raon_entry
#define VEC_SUFFIX raon_entry
#define VEC_IMPLEMENTATION
#include "vector.h"

struct raon_entry raon_parse_entry(struct raon_token first_token) { }

void raon_parse_block() { }

void raon_parse_value() { }

void raon_parse_array() { }

Vector_of_raon_entry raon_parse(char *str) {
   struct raon_lexer lex = raon_lexer_init(str);

   Vector_of_raon_entry *entries = vec_new_raon_entry();
   for (;;) {
      struct raon_token token = raon_lexer_eat(&lex);
      if (token.type == raon_token_type_error || token.type == raon_token_type_eof) {
         break;
      }

      struct raon_entry entry = raon_parse_entry(token);
      if (entry.value.type != raon_value_type_error) {
         vec_push_raon_entry(entries, entry);
      }
   }
}
