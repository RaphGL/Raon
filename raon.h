#ifndef RAON_H
#define RAON_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum raon_token_type {
  // symbols
  raon_token_type_equal,
  raon_token_type_newline,
  raon_token_type_comma,
  raon_token_type_block_open,
  raon_token_type_block_close,
  raon_token_type_array_open,
  raon_token_type_array_close,
  // value types
  raon_token_type_string,
  raon_token_type_field,
  raon_token_type_bool,
  raon_token_type_int,
  // used to indicate that an error ocurred
  raon_token_type_error,
};

struct raon_token {
  size_t from_x, from_y;
  size_t to_x, to_y;
  enum raon_token_type type;

  union {
    char *string_val;
    intptr_t int_val;
    char char_val;
    bool bool_val;
  };
};

struct raon_lexer {
  bool start;
  size_t curr_idx;
  size_t curr_x, curr_y;
  char *str;
  size_t str_len;
};

struct raon_parser {
  size_t curr_idx;
  struct raon_token_vec *tokens;
};

enum raon_value_type {
  raon_value_type_string,
  raon_value_type_int,
  raon_value_type_bool,
  raon_value_type_block,
  raon_value_type_error,
  raon_value_type_array,
};

struct raon_parser_value {
  enum raon_value_type type;
  union {
    char *string_val;
    intptr_t int_val;
    bool bool_val;
    struct raon_parser_vec *block_val;
    struct raon_value_vec *array_val;
  };
};

struct raon_parser_array {
  enum raon_value_type type;
  struct raon_value_vec *values;
};

enum raon_field_type {
  raon_field_type_string,
  raon_field_type_int,
  raon_field_type_error,
};

struct raon_parser_entry {
  enum raon_field_type field_type;
  union {
    char *string_field;
    intptr_t int_field;
  };
  struct raon_parser_value value;
};

// INTENDED PUBLIC API

struct raon_lexer raon_lexer_init_from_str(char *str);
struct raon_lexer raon_lexer_init_from_file(char *filepath);
struct raon_token_vec *raon_lexer_lex(struct raon_lexer *lex);
void raon_lexer_free(struct raon_lexer *lex);

struct raon_parser raon_parser_init(struct raon_token_vec *tokens);
struct raon_parser_vec *raon_parser_parse(struct raon_parser *parser);

struct raon_token_vec;
struct raon_parser_vec;

void raon_token_vec_free(struct raon_token_vec *vec);
void raon_parser_vec_free(struct raon_parser_vec *vec);

// MOSTLY USED INTERNALLY
char raon_lexer_peek(struct raon_lexer *lex);
void raon_lexer_eat(struct raon_lexer *lex);
struct raon_token raon_lexer_lex_number(struct raon_lexer *lex);
struct raon_token raon_lexer_lex_string(struct raon_lexer *lex);
struct raon_token raon_lexer_lex_ident(struct raon_lexer *lex);
struct raon_token raon_parser_peek(struct raon_parser *parser);
void raon_parser_eat(struct raon_parser *parser);
struct raon_parser_vec *raon_parser_parse_block(struct raon_parser *parser);
struct raon_parser_entry raon_parser_parse_entry(struct raon_parser *parser);
struct raon_parser_value raon_parser_parse_value(struct raon_parser *parser);
struct raon_value_vec *raon_parser_parse_array(struct raon_parser *parser);

#endif
