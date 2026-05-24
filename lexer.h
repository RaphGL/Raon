#ifndef RAON_LEXER_H
#define RAON_LEXER_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum raon_token_type {
   // symbols
   raon_token_type_equal,
   raon_token_type_newline,
   raon_token_type_comma,
   raon_token_type_block_open,
   raon_token_type_block_close,
   raon_token_type_array_open,
   raon_token_type_array_close,
   raon_token_type_eof,
   // value types
   raon_token_type_string,
   raon_token_type_field,
   raon_token_type_bool,
   raon_token_type_int,
   // used to indicate that an error ocurred
   raon_token_type_error,
};

struct raon_token {
   size_t start_line, start_col;
   size_t end_line, end_col;
   enum raon_token_type type;
   union {
      char *str_val;
      intptr_t int_val;
      char char_val;
      bool bool_val;
   };
};

struct raon_lexer {
   bool start;
   size_t line, col;
   size_t idx, str_len;
   char *str;
};

struct raon_lexer raon_lexer_init(char *str);
struct raon_token raon_lexer_eat(struct raon_lexer *self);

#endif
