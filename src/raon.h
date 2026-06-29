#ifndef RAON_H
#define RAON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// === String Slice ===

/*
   Stores the pointer to the beginning of a string and its length.
   If necessary one can convert it back to strings by either using
   `raon_str_from_slice`, `raon_slice_from_str` and `raon_copy_slice_to_str`
*/
struct raon_str_slice {
   char *ptr;
   size_t len;
};

/*
   Returns an allocated string with the contents of the slice.

   Returns: NULL if allocation failed

   Note: The user is responsible to free its memory.

*/
char *raon_str_from_slice(struct raon_str_slice slice);

/*
   Checks that the slice fits in the `str` buffer.

   Inputs:
   - `str`: start of the string slice
   - `len`: number of bytes after the `str` that are part of the slice

   Returns: `{0}` if slice overflows string

   Example:

   char *str[] = "hello world";
   // slice stores "world"
   struct raon_str_slice slice = raon_slice_from_str(&str[7], 5);
*/
struct raon_str_slice raon_slice_from_str(char *str, size_t len);

/*
   Copies the content of the slice to the buffer in dest.

   Returns: NULL if `size` is smaller than the length of the string slice
*/
char *raon_copy_slice_to_str(char *dest, size_t size, struct raon_str_slice src);

// === Lexer ===

enum raon_token_type {
   // symbols
   raon_token_type_equal,
   raon_token_type_newline,
   raon_token_type_comma,
   raon_token_type_dot,
   raon_token_type_block_open,
   raon_token_type_block_close,
   raon_token_type_array_open,
   raon_token_type_array_close,
   raon_token_type_eof,
   // value types
   raon_token_type_string,
   raon_token_type_key,
   raon_token_type_bool,
   raon_token_type_int,
   raon_token_type_float,
   // used to indicate that an error ocurred
   raon_token_type_error,
};

struct raon_token {
   size_t start_line, start_col;
   size_t end_line, end_col;
   enum raon_token_type type;
   union {
      struct raon_str_slice str_val;
      intptr_t int_val;
      char char_val;
      bool bool_val;
      double float_val;
   };
};

struct raon_lexer {
   size_t line, col;
   size_t idx, str_len;
   char *str;
};

/*
   Inputs:
   - `str`: string to start of contents that will be lexed
*/
struct raon_lexer raon_lexer_init(char *str, size_t len);

struct raon_token raon_lexer_eat(struct raon_lexer *self);
struct raon_token raon_lexer_lex_num(struct raon_lexer *self);
struct raon_token raon_lexer_lex_string(struct raon_lexer *self);
struct raon_token raon_lexer_lex_ident(struct raon_lexer *self);
void raon_lexer_ignore_comment(struct raon_lexer *self);

// === Parser ===

#define VEC_ITEM_TYPE struct raon_value
#define VEC_SUFFIX raon_value
#include "../vendor/vector.h"

#define VEC_ITEM_TYPE struct raon_entry
#define VEC_SUFFIX raon_entry
#include "../vendor/vector.h"

enum raon_value_type {
   raon_value_type_string,
   raon_value_type_int,
   raon_value_type_bool,
   raon_value_type_float,
   raon_value_type_block,
   raon_value_type_array,
   raon_value_type_error,
};

struct raon_value {
   enum raon_value_type type;
   union {
      struct raon_str_slice str_val;
      intptr_t int_val;
      bool bool_val;
      double float_val;
      struct vector_of_raon_entry *block_val;
      struct vector_of_raon_value *array_val;
   };
};

enum raon_key_type {
   raon_key_type_string,
   raon_key_type_num,
   raon_key_type_error,
};

struct raon_entry {
   enum raon_key_type key_type;
   union {
      struct raon_str_slice str_key;
      intptr_t int_key;
   };
   struct raon_value value;
};

struct raon_entry raon_parse_entry(struct vec_allocator allocator, struct raon_lexer *lexer, struct raon_token first_token);
struct raon_value raon_parse_value(struct vec_allocator allocator, struct raon_lexer *lexer, struct raon_token first_token);
struct vector_of_raon_value *raon_parse_array(struct vec_allocator allocator, struct raon_lexer *lexer, struct raon_token first_token);
struct vector_of_raon_entry *raon_parse_block(struct vec_allocator allocator, struct raon_lexer *lexer, struct raon_token first_token);

/*
   Parses text into a Raon AST.

   Inputs:
   - `str`: string buffer that should be parsed
   - `len`:size of `str`

   Returns: NULL if parsing failed
*/
struct vector_of_raon_entry *raon_parse(struct vec_allocator allocator, char *str, size_t len);

void raon_free_values(struct vector_of_raon_value *values);
void raon_free_entries(struct vector_of_raon_entry *entries);

/*
   Passed to print functions so that they can figure out how to properly do indentation
   and which whitespace indentation is preferred.
*/
struct raon_print_ctx {
   // indentation used per level
   char *indent;
   size_t indent_level;
};

void raon_print_entry(struct raon_print_ctx ctx, struct raon_entry entry);
void raon_print_value(struct raon_print_ctx ctx, struct raon_value value);
void raon_print_array(struct raon_print_ctx ctx, struct vector_of_raon_value *array);
void raon_print_entries(struct raon_print_ctx ctx, struct vector_of_raon_entry *entries);

#ifdef __cplusplus
}
#endif


#endif
