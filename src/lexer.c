#include "lexer.h"
#include "str_slice.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef raon_malloc
   #define raon_malloc malloc
#endif

#ifndef raon_free
   #define raon_free free
#endif

struct raon_lexer raon_lexer_init(char *str) {
   return (struct raon_lexer) { .str = str, .str_len = strlen(str), .start = false, .line = 1};
}

static char raon_lexer_peek_char(struct raon_lexer *self) {
   if (self->idx >= self->str_len) {
      return '\0';
   }
   return self->str[self->idx];
}

static char raon_lexer_eat_char(struct raon_lexer *self) {
   if (self->idx >= self->str_len) {
      return '\0';
   }

   char curr = self->str[self->idx];
   if (self->start) {
      ++self->idx;
   } else {
      self->start = true;
   }

   if (curr == '\n') {
      ++self->line;
      self->col = 0;
   } else {
      ++self->col;
   }
   return curr;
}

static void raon_lexer_ignore_comment(struct raon_lexer *self) {
   if (raon_lexer_peek_char(self) != '#') {
      return;
   }
   raon_lexer_eat_char(self);

   for (;;) {
      char curr = raon_lexer_peek_char(self);
      if (curr == '\0' || curr == '\n') {
         break;
      }
      raon_lexer_eat_char(self);
   }
}

static struct raon_token raon_lexer_lex_string(struct raon_lexer *self) {
   if (raon_lexer_peek_char(self) != '"') {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   raon_lexer_eat_char(self);

   struct raon_token token = {
      .type = raon_token_type_string,
      .start_line = self->line,
      .start_col = self->col,
   };

   const size_t start_str = self->idx;
   while (raon_lexer_peek_char(self) != '\0') {
      char curr = raon_lexer_eat_char(self);
      if (curr == '"') {
         break;
      }
   }
   const size_t end_str = self->idx - 1;
   const size_t str_len = end_str - start_str;
   token.end_line = self->line;
   token.end_col = self->col;
   token.str_val = raon_slice_from_str(&self->str[start_str], str_len);
   if (!token.str_val.ptr) {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   return token;
}

static struct raon_token raon_lexer_lex_int(struct raon_lexer *self) {
   char curr = raon_lexer_peek_char(self);
   if (!isdigit(curr) && curr != '-') {
      return (struct raon_token) { .type = raon_token_type_error };
   }

   struct raon_token token = {
      .type = raon_token_type_int,
      .start_line = self->line,
      .start_col = self->col,
   };

   size_t start_int = self->idx;
   if (raon_lexer_peek_char(self) == '-') {
      raon_lexer_eat_char(self);
   }
   while (isdigit(raon_lexer_peek_char(self))) {
      raon_lexer_eat_char(self);
   }
   size_t end_int = self->idx;
   size_t int_len = end_int - start_int + 1;
   token.end_col = self->col;
   token.end_line = self->line;

   char *int_str = raon_malloc(int_len + 1);
   if (!int_str) {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   strncpy(int_str, &self->str[start_int], int_len);
   int_str[int_len] = '\0';

   errno = 0;
   token.int_val = strtoll(int_str, NULL, 10);
   if (errno == EINVAL || errno == ERANGE) {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   raon_free(int_str);
   return token;
}

static struct raon_token raon_lexer_lex_ident(struct raon_lexer *self) {
   if (!isalpha(raon_lexer_peek_char(self))) {
      return (struct raon_token) { .type = raon_token_type_error };
   }

   struct raon_token token = {
      .type = raon_token_type_key,
      .start_line = self->line,
      .start_col = self->col,
   };

   size_t start_ident = self->idx;
   for (;;) {
      char c = raon_lexer_peek_char(self);
      if (c != '_' && c != '-' && !isalnum(c)) {
         break;
      }
      raon_lexer_eat_char(self);
   }
   size_t end_ident = self->idx - 1;
   size_t ident_len = end_ident - start_ident + 1;
   char *ident = &self->str[start_ident];

   if (strncmp(ident, "true", ident_len) == 0) {
      token.type = raon_token_type_bool;
      token.bool_val = true;
      return token;
   }

   if (strncmp(ident, "false", ident_len) == 0) {
      token.type = raon_token_type_bool;
      token.bool_val = false;
      return token;
   }

   token.type = raon_token_type_key;
   token.str_val = raon_slice_from_str(ident, ident_len);
   if (!token.str_val.ptr) {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   return token;
}

struct raon_token raon_lexer_eat(struct raon_lexer *self) {
#define RAON_ONE_CHAR_TOKEN(literal, enum_type)                                                    \
   (struct raon_token) {                                                                           \
      .type = enum_type, .char_val = literal, .start_col = self->col, .start_line = self->line,    \
      .end_line = self->line, .end_col = self->col,                                                \
   }

   struct raon_token curr_token = { 0 };
   for (;;) {
      char curr = raon_lexer_peek_char(self);
      if (curr == '\0') {
         break;
      }

      switch (curr) {
      case '#':
         raon_lexer_ignore_comment(self);
         break;

      case '\n':
         raon_lexer_eat_char(self);
         return RAON_ONE_CHAR_TOKEN('\n', raon_token_type_newline);

      case '=':
         raon_lexer_eat_char(self);
         return RAON_ONE_CHAR_TOKEN('=', raon_token_type_equal);

      case '{':
         raon_lexer_eat_char(self);
         return RAON_ONE_CHAR_TOKEN('{', raon_token_type_block_open);

      case '}':
         raon_lexer_eat_char(self);
         return RAON_ONE_CHAR_TOKEN('}', raon_token_type_block_close);

      case '[':
         raon_lexer_eat_char(self);
         return RAON_ONE_CHAR_TOKEN('[', raon_token_type_array_open);

      case ']':
         raon_lexer_eat_char(self);
         return RAON_ONE_CHAR_TOKEN(']', raon_token_type_array_close);

      case ',':
         raon_lexer_eat_char(self);
         return RAON_ONE_CHAR_TOKEN(',', raon_token_type_comma);

      case '.':
         raon_lexer_eat_char(self);
         return RAON_ONE_CHAR_TOKEN('.', raon_token_type_dot);
      }

      if (isspace(curr)) {
         raon_lexer_eat_char(self);
         continue;
      }

      curr_token = raon_lexer_lex_ident(self);
      if (curr_token.type != raon_token_type_error) {
         return curr_token;
      }

      curr_token = raon_lexer_lex_int(self);
      if (curr_token.type != raon_token_type_error) {
         return curr_token;
      }

      curr_token = raon_lexer_lex_string(self);
      if (curr_token.type != raon_token_type_error) {
         return curr_token;
      }
   }

   return RAON_ONE_CHAR_TOKEN('\0', raon_token_type_eof);
#undef RAON_ONE_CHAR_TOKEN
}
