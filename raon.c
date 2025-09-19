#include "raon.h"
#include <asm-generic/errno-base.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef raon_malloc
   #define raon_malloc malloc
#endif

#ifndef raon_free
   #define raon_free free
#endif

struct raon_lexer raon_lexer_init_from_str(char *str) {
   const size_t str_len = strlen(str);
   char *str_copy = raon_malloc(str_len * sizeof(char));
   strncpy(str_copy, str, str_len);
   return (struct raon_lexer) {
      .str = str_copy,
      .str_len = str_len,
   };
}

// initialization failed if str field is NULL
struct raon_lexer raon_lexer_init_from_file(char *filepath) {
   FILE *file = fopen(filepath, "r");
   const struct raon_lexer error_val = { .str = NULL };
   if (!file) {
      return error_val;
   }

   fseek(file, 0, SEEK_END);
   size_t len = ftell(file);
   fseek(file, 0, SEEK_SET);

   char *str = raon_malloc(len + 1);
   if (!str) {
      fclose(file);
      return error_val;
   }
   fread(str, 1, len, file);
   fclose(file);
   str[len] = '\0';

   return (struct raon_lexer) { .str = str, .str_len = strlen(str) };
}

void raon_lexer_free(struct raon_lexer *lex) {
   raon_free(lex->str);
   *lex = (struct raon_lexer) { 0 };
}

char raon_lexer_peek(struct raon_lexer *lex) {
   size_t next_tok = lex->start ? lex->curr_idx + 1 : 0;

   if (next_tok >= lex->str_len) {
      return '\0';
   }
   return lex->str[next_tok];
}

void raon_lexer_eat(struct raon_lexer *lex) {
   size_t next_idx;
   if (lex->start) {
      next_idx = lex->curr_idx + 1;
   } else {
      next_idx = 0;
      lex->start = true;
   }
   if (next_idx >= lex->str_len) {
      return;
   }
   lex->curr_idx = next_idx;
   char next_tok = lex->str[next_idx];
   if (next_tok == '\n') {
      ++lex->curr_y;
      lex->curr_x = 0;
   } else {
      ++lex->curr_x;
   }
}

struct raon_token raon_lexer_lex_number(struct raon_lexer *lex) {
   const struct raon_token error_val = { .type = raon_token_type_error };
   if (!isdigit(raon_lexer_peek(lex))) {
      return error_val;
   }
   raon_lexer_eat(lex);

   struct raon_token tok = {
      .from_x = lex->curr_x,
      .from_y = lex->curr_y,
      .type = raon_token_type_int,
   };

   size_t str_start_idx = lex->curr_idx;
   size_t str_len = 1;
   for (;;) {
      char c = raon_lexer_peek(lex);
      if (!isdigit(c)) {
         break;
      }
      raon_lexer_eat(lex);
      ++str_len;
   }
   tok.to_x = lex->curr_x;
   tok.to_y = lex->curr_y;

   char *num = raon_malloc(str_len + 1);
   if (!num) {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   strncpy(num, &lex->str[str_start_idx], str_len);
   num[str_len] = '\0';

   errno = 0;
   tok.int_val = strtoll(num, NULL, 10);
   if (errno == EINVAL || errno == ERANGE) {
      return error_val;
   }
   raon_free(num);
   return tok;
}

struct raon_token raon_lexer_lex_string(struct raon_lexer *lex) {
   const struct raon_token error_val = (struct raon_token) { .type = raon_token_type_error };

   if (raon_lexer_peek(lex) != '"') {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   raon_lexer_eat(lex);
   raon_lexer_eat(lex);
   size_t str_start_idx = lex->curr_idx;

   struct raon_token tok = {
      .from_x = lex->curr_x,
      .from_y = lex->curr_y,
      .type = raon_token_type_string,
   };

   size_t str_len = 0;
   for (;;) {
      char c = raon_lexer_peek(lex);
      ++str_len;
      raon_lexer_eat(lex);
      if (c == '"') {
         break;
      }

      if (c == '\0') {
         return error_val;
      }
   }
   tok.string_val = raon_malloc(str_len + 1);
   if (!tok.string_val) {
      return error_val;
   }
   strncpy(tok.string_val, &lex->str[str_start_idx], str_len);
   tok.string_val[str_len] = '\0';
   return tok;
}

// lexes bools and fields
struct raon_token raon_lexer_lex_ident(struct raon_lexer *lex) {
   const struct raon_token error_val = {
      .type = raon_token_type_error,
   };

   if (!isalpha(raon_lexer_peek(lex))) {
      return error_val;
   }
   raon_lexer_eat(lex);

   struct raon_token tok = {
      .from_x = lex->curr_x,
      .from_y = lex->curr_y,
   };

   size_t str_start_idx = lex->curr_idx;
   size_t str_len = 0;

   for (;;) {
      char c = raon_lexer_peek(lex);
      ++str_len;
      raon_lexer_eat(lex);
      if (c != '_' && c != '-' && !isalnum(c)) {
         break;
      }
   }
   tok.to_x = lex->curr_x;
   tok.to_y = lex->curr_y;

   const char *str = &lex->str[str_start_idx];
   if (strncmp(str, "true", str_len) == 0) {
      tok.type = raon_token_type_bool;
      tok.bool_val = true;
      return tok;
   }

   if (strncmp(str, "false", str_len) == 0) {
      tok.type = raon_token_type_bool;
      tok.bool_val = false;
      return tok;
   }

   tok.type = raon_token_type_field;
   tok.string_val = raon_malloc(str_len + 1);
   if (!tok.string_val) {
      return error_val;
   }
   strncpy(tok.string_val, &lex->str[str_start_idx], str_len);
   tok.string_val[str_len] = '\0';
   return tok;
}

void raon_lexer_ignore_comment(struct raon_lexer *lex) {
   if (raon_lexer_peek(lex) != '#') {
      return;
   }
   raon_lexer_eat(lex);

   while (raon_lexer_peek(lex) != '\n') {
      raon_lexer_eat(lex);
   }
}

void raon_token_free(struct raon_token *token) {
   if (token->type == raon_token_type_string || token->type == raon_token_type_field) {
      raon_free(token->string_val);
   }

   *token = (struct raon_token) { 0 };
}

#define RAON_VEC(func_prefix, vec_item_type, vec_item_type_free)                                   \
   struct raon_##func_prefix##_vec {                                                               \
      vec_item_type *items;                                                                        \
      size_t cap, len;                                                                             \
   };                                                                                              \
                                                                                                   \
   struct raon_##func_prefix##_vec *raon_##func_prefix##_vec_init(void) {                          \
      const size_t cap = 64;                                                                       \
      vec_item_type *items = raon_malloc(cap * sizeof(*items));                                    \
      if (!items) {                                                                                \
         return NULL;                                                                              \
      }                                                                                            \
      memset(items, 0, cap);                                                                       \
                                                                                                   \
      struct raon_##func_prefix##_vec *vec = raon_malloc(sizeof(*vec));                            \
      if (!vec) {                                                                                  \
         return NULL;                                                                              \
      }                                                                                            \
      *vec = (struct raon_##func_prefix##_vec) {                                                   \
         .items = items,                                                                           \
         .cap = cap,                                                                               \
         .len = 0,                                                                                 \
      };                                                                                           \
                                                                                                   \
      return vec;                                                                                  \
   }                                                                                               \
                                                                                                   \
   void raon_##func_prefix##_vec_free(struct raon_##func_prefix##_vec *vec) {                      \
      for (size_t i = 0; i < vec->len; i++) {                                                      \
         vec_item_type_free(&vec->items[i]);                                                       \
         vec->items[i] = (vec_item_type) { 0 };                                                    \
      }                                                                                            \
      raon_free(vec->items);                                                                       \
      raon_free(vec);                                                                              \
   }                                                                                               \
                                                                                                   \
   bool raon_##func_prefix##_vec_push(struct raon_##func_prefix##_vec *vec, vec_item_type tok) {   \
      if (vec->len >= vec->cap) {                                                                  \
         vec->cap *= 2;                                                                            \
         vec_item_type *new_items = raon_malloc(vec->cap * sizeof(*new_items));                    \
         if (!new_items) {                                                                         \
            return false;                                                                          \
         }                                                                                         \
         memcpy(new_items, vec->items, vec->len * sizeof(*new_items));                             \
         raon_free(vec->items);                                                                    \
         vec->items = new_items;                                                                   \
      }                                                                                            \
                                                                                                   \
      vec->items[vec->len] = tok;                                                                  \
      ++vec->len;                                                                                  \
      return true;                                                                                 \
   }

static void raon_parser_value_free(struct raon_parser_value *value);
static void raon_parser_entry_free(struct raon_parser_entry *entry);
RAON_VEC(token, struct raon_token, raon_token_free)
RAON_VEC(value, struct raon_parser_value, raon_parser_value_free)
RAON_VEC(parser, struct raon_parser_entry, raon_parser_entry_free)

static void raon_parser_value_free(struct raon_parser_value *value) {
   switch (value->type) {
   case raon_value_type_string:
      raon_free(value->string_val);
      break;
   case raon_value_type_block:
      raon_parser_vec_free(value->block_val);
      break;

   case raon_value_type_array:
      raon_value_vec_free(value->array_val);
      break;

   default:
      // the other values don't allocate memory
      break;
   }
}

static void raon_parser_entry_free(struct raon_parser_entry *entry) {
   raon_free(entry->field);
   raon_parser_value_free(&entry->value);
}

struct raon_token_vec *raon_lexer_lex(struct raon_lexer *lex) {
#define RAON_ONE_CHAR_TOKEN(literal, enum_type)                                                    \
   (struct raon_token) {                                                                           \
      .type = enum_type, .char_val = literal, .from_x = lex->curr_x, .from_y = lex->curr_y,        \
      .to_x = lex->curr_x, .to_y = lex->curr_y,                                                    \
   }

   struct raon_token_vec *token_vec = raon_token_vec_init();
   if (!token_vec) {
      return NULL;
   }

   while (lex->curr_idx < lex->str_len) {
      raon_lexer_ignore_comment(lex);

      char c = raon_lexer_peek(lex);
      if (c == '\0') {
         break;
      }

      struct raon_token curr_token = { 0 };
      if (c == '\n') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('\n', raon_token_type_newline);
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      if (isspace(c)) {
         raon_lexer_eat(lex);
         continue;
      }

      curr_token = raon_lexer_lex_ident(lex);
      if (curr_token.type != raon_token_type_error) {
         if (!raon_token_vec_push(token_vec, curr_token)) {
            raon_token_free(&curr_token);
            return NULL;
         }
         continue;
      }

      curr_token = raon_lexer_lex_number(lex);
      if (curr_token.type != raon_token_type_error) {
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      curr_token = raon_lexer_lex_string(lex);
      if (curr_token.type != raon_token_type_error) {
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      if (c == '=') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('=', raon_token_type_equal);
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      if (c == '{') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('{', raon_token_type_block_open);
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      if (c == '}') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('}', raon_token_type_block_close);
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      if (c == '[') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('[', raon_token_type_array_open);
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      if (c == ']') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN(']', raon_token_type_array_close);
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      if (c == ',') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN(',', raon_token_type_comma);
         if (!raon_token_vec_push(token_vec, curr_token)) {
            return NULL;
         }
         continue;
      }

      raon_token_vec_free(token_vec);
      token_vec = NULL;
      break;
   }

#undef RAON_ONE_CHAR_TOKEN
   return token_vec;
}

struct raon_parser raon_parser_init(struct raon_token_vec *tokens) {
   return (struct raon_parser) {
      .tokens = tokens,
   };
}

// returns error when there's nothing more to peek
struct raon_token raon_parser_peek(struct raon_parser *parser) {
   const struct raon_token error_val = {
      .type = raon_token_type_error,
   };

   if (parser->curr_idx >= parser->tokens->len) {
      return error_val;
   }
   return parser->tokens->items[parser->curr_idx];
}

void raon_parser_eat(struct raon_parser *parser) {
   if (parser->curr_idx >= parser->tokens->len) {
      return;
   }
   ++parser->curr_idx;
}

struct raon_parser_vec *raon_parser_parse_block(struct raon_parser *parser) {
   struct raon_token tok = raon_parser_peek(parser);
   if (tok.type != raon_token_type_block_open) {
      return NULL;
   }
   raon_parser_eat(parser);

   struct raon_parser_vec *entries = raon_parser_vec_init();

   for (struct raon_token tok = raon_parser_peek(parser); tok.type != raon_token_type_error;
       tok = raon_parser_peek(parser)) {
      if (tok.type == raon_token_type_block_close) {
         raon_parser_eat(parser);
         break;
      }

      if (tok.type == raon_token_type_newline) {
         raon_parser_eat(parser);
         continue;
      }

      struct raon_parser_entry entry = raon_parser_parse_entry(parser);
      if (entry.value.type == raon_value_type_error) {
         raon_parser_vec_free(entries);
         return NULL;
      }
      if (!raon_parser_vec_push(entries, entry)) {
         raon_parser_vec_free(entries);
         return NULL;
      }
   }

   return entries;
}

struct raon_parser_value raon_parser_parse_value(struct raon_parser *parser) {
   struct raon_parser_value error_val = { .type = raon_value_type_error };
   struct raon_parser_value value = { 0 };

   struct raon_token curr_token = raon_parser_peek(parser);
   switch (curr_token.type) {
   case raon_token_type_string: {
      const size_t strsiz = strlen(curr_token.string_val);
      value.string_val = raon_malloc(strsiz + 1);
      if (!value.string_val) {
         return error_val;
      }
      strncpy(value.string_val, curr_token.string_val, strsiz);
      value.string_val[strsiz] = '\0';
      value.type = raon_value_type_string;
      break;
   }

   case raon_token_type_bool:
      value.bool_val = curr_token.bool_val;
      value.type = raon_value_type_bool;
      break;

   case raon_token_type_int:
      value.int_val = curr_token.int_val;
      value.type = raon_value_type_int;
      break;

   case raon_token_type_block_open: {
      struct raon_parser_vec *block = raon_parser_parse_block(parser);
      if (!block) {
         return error_val;
      }
      value.block_val = block;
      value.type = raon_value_type_block;
      break;
   }

   case raon_token_type_array_open: {
      struct raon_value_vec *array = raon_parser_parse_array(parser);
      if (!array) {
         return error_val;
      }
      value.array_val = array;
      value.type = raon_value_type_array;
      break;
   }

   default:
      return error_val;
   }
   raon_parser_eat(parser);

   curr_token = raon_parser_peek(parser);
   if (curr_token.type == raon_token_type_comma || curr_token.type == raon_token_type_newline) {
      raon_parser_eat(parser);
   }

   return value;
}

struct raon_value_vec *raon_parser_parse_array(struct raon_parser *parser) {
   if (raon_parser_peek(parser).type != raon_token_type_array_open) {
      return NULL;
   }
   raon_parser_eat(parser);

   struct raon_value_vec *values = raon_value_vec_init();
   // we start with an error as our "zero value"
   // the first type we see in the array, we consider it as our array's type
   enum raon_value_type array_type = raon_value_type_error;

   for (struct raon_token tok = raon_parser_peek(parser); tok.type != raon_token_type_error;
       tok = raon_parser_peek(parser)) {
      if (tok.type == raon_token_type_newline) {
         raon_parser_eat(parser);
         continue;
      }

      if (tok.type == raon_token_type_array_close) {
         raon_parser_eat(parser);
         break;
      }

      struct raon_parser_value curr_value = raon_parser_parse_value(parser);
      if (curr_value.type == raon_value_type_error) {
         raon_value_vec_free(values);
         return NULL;
      }
      if (array_type == raon_value_type_error) {
         array_type = curr_value.type;
      }
      if (curr_value.type != array_type) {
         raon_value_vec_free(values);
         raon_parser_value_free(&curr_value);
         return NULL;
      }
      raon_value_vec_push(values, curr_value);
   }

   return values;
}

struct raon_parser_entry raon_parser_parse_entry(struct raon_parser *parser) {
   struct raon_parser_entry error_val = { .value.type = raon_value_type_error };
   struct raon_parser_entry entry = { 0 };

   struct raon_token curr_token = raon_parser_peek(parser);
   if (curr_token.type != raon_token_type_field) {
      return error_val;
   }
   const size_t strsiz = strlen(curr_token.string_val);
   entry.field = raon_malloc(strsiz + 1);
   if (!entry.field) {
      return error_val;
   }
   strncpy(entry.field, curr_token.string_val, strsiz);
   entry.field[strsiz] = '\0';
   raon_parser_eat(parser);

   curr_token = raon_parser_peek(parser);
   if (curr_token.type != raon_token_type_equal) {
      raon_free(entry.field);
      return error_val;
   }
   raon_parser_eat(parser);

   entry.value = raon_parser_parse_value(parser);
   if (entry.value.type == raon_value_type_error) {
      raon_free(entry.field);
      return error_val;
   }

   return entry;
}

struct raon_parser_vec *raon_parser_parse(struct raon_parser *parser) {
   struct raon_parser_vec *entries = raon_parser_vec_init();

   for (struct raon_token tok = raon_parser_peek(parser); tok.type != raon_token_type_error;
       tok = raon_parser_peek(parser)) {
      if (tok.type == raon_token_type_newline) {
         raon_parser_eat(parser);
         continue;
      }

      struct raon_parser_entry entry = raon_parser_parse_entry(parser);
      if (entry.value.type == raon_value_type_error) {
         raon_parser_vec_free(entries);
         return NULL;
      }
      if (!raon_parser_vec_push(entries, entry)) {
         raon_parser_vec_free(entries);
         return NULL;
      }
   }

   return entries;
}

int main(void) {
   struct raon_lexer lex = raon_lexer_init_from_file("./example.raon");
   if (!lex.str) {
      perror("reading file failed");
      goto cleanup;
   }

   puts(lex.str);

   struct raon_token_vec *tokens = raon_lexer_lex(&lex);
   if (!tokens) {
      puts("error lexing");
      goto cleanup;
   }

   struct raon_parser parser = raon_parser_init(tokens);
   struct raon_parser_vec *entries = raon_parser_parse(&parser);
   if (!entries) {
      puts("error parsing");
      goto cleanup;
   }

   puts("==> finished parsing with no errors");

cleanup:
   if (entries)
      raon_parser_vec_free(entries);
   if (tokens)
      raon_token_vec_free(tokens);
   if (lex.str != NULL)
      raon_lexer_free(&lex);

   return 0;
}
