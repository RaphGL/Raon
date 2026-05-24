#ifndef RAON_PARSER_H
#define RAON_PARSER_H

#include "lexer.h"
#include <stdbool.h>
#include <stdint.h>

#define VEC_ITEM_TYPE struct raon_value
#define VEC_SUFFIX raon_value
#include "vendor/vector.h"

#define VEC_ITEM_TYPE struct raon_entry
#define VEC_SUFFIX raon_entry
#include "vendor/vector.h"

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
      struct vector_of_raon_entry *block_val;
      struct vector_of_raon_value *array_val;
   };
};

enum raon_key_type {
   raon_field_type_string,
   raon_field_type_int,
   raon_field_type_error,
};

struct raon_entry {
   enum raon_key_type field_type;
   union {
      char *str_field;
      intptr_t int_field;
   };
   struct raon_value value;
};

struct raon_entry raon_parse_entry(struct raon_lexer *lexer, struct raon_token first_token);
struct raon_value raon_parse_value(struct raon_lexer *lexer, struct raon_token first_token);
struct vector_of_raon_value *raon_parse_array(struct raon_lexer *lexer, struct raon_token first_token);
struct vector_of_raon_entry *raon_parse_block(struct raon_lexer *lexer, struct raon_token first_token);
struct vector_of_raon_entry *raon_parse(char *str);

#endif
