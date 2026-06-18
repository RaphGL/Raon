#include "raon.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct raon_lexer raon_lexer_init(char *str, size_t len) {
   return (struct raon_lexer) { .str = str, .str_len = len, .line = 1 };
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
   ++self->idx;

   if (curr == '\n') {
      ++self->line;
      self->col = 0;
   } else {
      ++self->col;
   }
   return curr;
}

void raon_lexer_ignore_comment(struct raon_lexer *self) {
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

struct raon_token raon_lexer_lex_string(struct raon_lexer *self) {
   struct raon_token error_val = { .type = raon_token_type_error };
   if (raon_lexer_peek_char(self) != '"') {
      return error_val;
   }
   raon_lexer_eat_char(self);

   struct raon_token token = {
      .type = raon_token_type_error,
      .start_line = self->line,
      .start_col = self->col,
   };

   const size_t start_str = self->idx;
   while (raon_lexer_peek_char(self) != '\0') {
      char curr = raon_lexer_eat_char(self);
      if (curr == '"') {
         token.type = raon_token_type_string;
         break;
      }
   }
   const size_t end_str = self->idx - 1;
   const size_t str_len = end_str - start_str;
   token.end_line = self->line;
   token.end_col = self->col;
   token.str_val = raon_slice_from_str(&self->str[start_str], str_len);
   if (!token.str_val.ptr) {
      return error_val;
   }
   return token;
}

static struct raon_token raon_lexer_lex_scientific_num(struct raon_lexer *self) {
   // TODO
}

struct raon_token raon_lexer_lex_num(struct raon_lexer *self) {
   struct raon_token error_val = { .type = raon_token_type_error };
   char curr = raon_lexer_peek_char(self);
   if (!isdigit(curr) && curr != '-') {
      return error_val;
   }

   struct raon_token token = {
      .type = raon_token_type_int,
      .start_line = self->line,
      .start_col = self->col,
   };

   size_t start_int = self->idx;

   enum num_type {
      num_type_hex,
      num_type_decimal,
      num_type_octal,
      num_type_binary,
      num_type_float,
   } int_type = num_type_decimal;

   curr = raon_lexer_peek_char(self);
   if (curr == '-') {
      raon_lexer_eat_char(self);
   } else if (curr == '0') {
      raon_lexer_eat_char(self);
      curr = raon_lexer_peek_char(self);

      if (curr == 'b' || curr == 'o' || curr == 'x') {
         raon_lexer_eat_char(self);
         switch (curr) {
         case 'b':
            int_type = num_type_binary;
            raon_lexer_eat_char(self);
            break;

         case 'o':
            int_type = num_type_octal;
            raon_lexer_eat_char(self);
            break;

         case 'x':
            int_type = num_type_hex;
            raon_lexer_eat_char(self);
            break;
         }
      }
   }

   size_t dot_count = 0;

   for (char next = raon_lexer_peek_char(self); isdigit(next) || next == '_'
       || (next >= 'A' && next <= 'F' || next >= 'a' && next <= 'f' || next == '.');
       next = raon_lexer_peek_char(self)) {
      if (raon_lexer_eat_char(self) == '.') {
         ++dot_count;
      }
   }

   // floating points can only have one dot
   if (dot_count > 1) {
      return error_val;
   }
   if (dot_count == 1) {
      int_type = num_type_float;
      token.type = raon_token_type_float;
   }

   // trailing number separators are not allowed
   if (self->str[self->idx - 1] == '_') {
      return error_val;
   }

   size_t end_int = self->idx;
   size_t int_len = end_int - start_int + 1;
   token.end_col = self->col;
   token.end_line = self->line;

   char *num_str = malloc(int_len + 1);
   if (!num_str) {
      return error_val;
   }

   // ignore number separators
   char *int_slice = &self->str[start_int];
   size_t int_str_idx = 0;
   for (size_t i = 0; i < int_len; i++) {
      if (int_slice[i] == '_') {
         continue;
      }
      num_str[int_str_idx++] = int_slice[i];
   }
   num_str[int_len] = '\0';

   errno = 0;
   switch (int_type) {
   case num_type_binary:
      token.int_val = strtoll(num_str + 2, NULL, 2);
      break;

   case num_type_octal:
      token.int_val = strtoll(num_str + 2, NULL, 8);
      break;

   case num_type_decimal:
      token.int_val = strtoll(num_str, NULL, 10);
      break;

   case num_type_hex:
      token.int_val = strtoll(num_str + 2, NULL, 16);
      break;

   case num_type_float:
      token.float_val = strtod(num_str, NULL);
      break;
   }
   if (errno == EINVAL || errno == ERANGE) {
      return error_val;
   }
   free(num_str);
   return token;
}

struct raon_token raon_lexer_lex_ident(struct raon_lexer *self) {
   char start_char = raon_lexer_peek_char(self);
   if (!isalpha(start_char) && start_char != '_') {
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

      curr_token = raon_lexer_lex_num(self);
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
