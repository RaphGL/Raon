#include <ctype.h>
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

enum raon_token_type {
   raon_token_type_field,
   raon_token_type_equal,
   raon_token_type_block_open,
   raon_token_type_block_close,
   raon_token_type_newline,
   raon_token_type_comma,
   // value types
   raon_token_type_string,
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
      char* string_val;
      int int_val;
      char char_val;
      bool bool_val;
   };
};

struct raon_lexer {
   bool start;
   size_t curr_idx;
   size_t curr_x, curr_y;
   char* str;
   size_t str_len;
};

struct raon_lexer raon_lexer_init_from_str(char* str) {
   const size_t str_len = strlen(str);
   char* str_copy = raon_malloc(str_len * sizeof(char));
   strncpy(str_copy, str, str_len);
   return (struct raon_lexer) {
      .str = str_copy,
      .str_len = str_len,
   };
}

// initialization failed if str field is NULL
struct raon_lexer raon_lexer_init_from_file(FILE* file) {
   if (!file) {
      return (struct raon_lexer) {
         .str = NULL,
      };
   }

   fseek(file, 0, SEEK_END);
   size_t len = ftell(file);
   fseek(file, 0, SEEK_SET);

   char* str = raon_malloc(len + 1);
   if (str) {
      fread(str, 1, len, file);
   }
   str[len] = '\0';
   fclose(file);

   return (struct raon_lexer) { .str = str, .str_len = strlen(str) };
}

void raon_lexer_free(struct raon_lexer* lex) {
   raon_free(lex->str);
   lex->str = NULL;
}

char raon_lexer_peek(struct raon_lexer* lex) {
   size_t next_tok = lex->start ? lex->curr_idx + 1 : 0;

   if (next_tok >= lex->str_len) {
      return '\0';
   }
   return lex->str[next_tok];
}

void raon_lexer_eat(struct raon_lexer* lex) {
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

struct raon_token raon_lexer_lex_number(struct raon_lexer* lex) {
   if (!isdigit(raon_lexer_peek(lex))) {
      return (struct raon_token) { .type = raon_token_type_error };
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

   char* num = raon_malloc(str_len + 1);
   if (!num) {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   strncpy(num, &lex->str[str_start_idx], str_len);
   num[str_len] = '\0';
   // TODO: handle error on strtoll
   tok.int_val = strtoll(num, NULL, 10);
   raon_free(num);
   return tok;
}

struct raon_token raon_lexer_lex_string(struct raon_lexer* lex) {
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
   }
   tok.string_val = raon_malloc(str_len + 1);
   tok.string_val[str_len] = '\0';
   if (!tok.string_val) {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   strncpy(tok.string_val, &lex->str[str_start_idx], str_len);
   return tok;
}

// lexes bools and fields
struct raon_token raon_lexer_lex_ident(struct raon_lexer* lex) {
   if (!isalpha(raon_lexer_peek(lex))) {
      return (struct raon_token) { .type = raon_token_type_error };
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

   tok.string_val = raon_malloc(str_len + 1);
   tok.string_val[str_len] = '\0';
   if (!tok.string_val) {
      return (struct raon_token) { .type = raon_token_type_error };
   }
   strncpy(tok.string_val, &lex->str[str_start_idx], str_len);

   if (strcmp(tok.string_val, "true") == 0 || strcmp(tok.string_val, "false") == 0) {
      tok.type = raon_token_type_bool;
   } else {
      tok.type = raon_token_type_field;
   }
   return tok;
}

void raon_token_free(struct raon_token* token) {
   switch (token->type) {
   case raon_token_type_string:
   case raon_token_type_field:
   case raon_token_type_bool:
      raon_free(token->string_val);
      break;

   default:
      // do nothing as these can be stored without allocation
      break;
   }

   *token = (struct raon_token) { 0 };
}

struct raon_token_vec {
   struct raon_token* tokens;
   size_t cap, len;
};

struct raon_token_vec* raon_token_vec_init(void) {
   const size_t cap = 64;
   struct raon_token* tokens = raon_malloc(cap * sizeof(*tokens));
   if (!tokens) {
      return NULL;
   }
   memset(tokens, 0, cap);

   struct raon_token_vec* vec = raon_malloc(sizeof(*vec));
   if (!vec) {
      return NULL;
   }
   *vec = (struct raon_token_vec) {
      .tokens = tokens,
      .cap = cap,
      .len = 0,
   };

   return vec;
}

void raon_token_vec_free(struct raon_token_vec* vec) {
   for (size_t i = 0; i < vec->len; i++) {
      raon_token_free(&vec->tokens[i]);
   }
   raon_free(vec->tokens);
   raon_free(vec);
}

bool raon_token_vec_push(struct raon_token_vec* vec, struct raon_token tok) {
   if (vec->len >= vec->cap) {
      vec->cap *= 2;
      struct raon_token* new_tokens = raon_malloc(vec->cap * sizeof(*new_tokens));
      if (!new_tokens) {
         return false;
      }
      memcpy(new_tokens, vec->tokens, vec->len * sizeof(*new_tokens));
      raon_free(vec->tokens);
      vec->tokens = new_tokens;
   }

   vec->tokens[vec->len] = tok;
   ++vec->len;
   return true;
}

struct raon_token_vec* raon_lexer_lex(struct raon_lexer* lex, size_t* token_len) {
#define RAON_ONE_CHAR_TOKEN(literal, enum_type)                                                    \
   (struct raon_token) {                                                                           \
      .type = enum_type, .char_val = literal, .from_x = lex->curr_x, .from_y = lex->curr_y,        \
      .to_x = lex->curr_x, .to_y = lex->curr_y,                                                    \
   }

   struct raon_token_vec* token_vec = raon_token_vec_init();
   if (!token_vec) {
      return NULL;
   }

   while (lex->curr_idx < lex->str_len) {
      char c = raon_lexer_peek(lex);
      if (c == '\0') {
         break;
      }

      struct raon_token curr_token = { 0 };
      if (c == '\n') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('\n', raon_token_type_newline);
         raon_token_vec_push(token_vec, curr_token);
         continue;
      }

      if (isspace(c)) {
         raon_lexer_eat(lex);
         continue;
      }

      curr_token = raon_lexer_lex_ident(lex);
      if (curr_token.type != raon_token_type_error) {
         raon_token_vec_push(token_vec, curr_token);
         continue;
      }

      curr_token = raon_lexer_lex_number(lex);
      if (curr_token.type != raon_token_type_error) {
         raon_token_vec_push(token_vec, curr_token);
         continue;
      }

      curr_token = raon_lexer_lex_string(lex);
      if (curr_token.type != raon_token_type_error) {
         raon_token_vec_push(token_vec, curr_token);
         continue;
      }

      if (c == '=') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('=', raon_token_type_equal);
         raon_token_vec_push(token_vec, curr_token);
         continue;
      }

      if (c == '{') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('{', raon_token_type_block_open);
         raon_token_vec_push(token_vec, curr_token);
         continue;
      }

      if (c == '}') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN('}', raon_token_type_block_close);
         raon_token_vec_push(token_vec, curr_token);
         continue;
      }

      if (c == ',') {
         raon_lexer_eat(lex);
         curr_token = RAON_ONE_CHAR_TOKEN(',', raon_token_type_comma);
         raon_token_vec_push(token_vec, curr_token);
         continue;
      }

      raon_token_vec_free(token_vec);
      token_vec = NULL;
      break;
   }

#undef RAON_ONE_CHAR_TOKEN
   return token_vec;
}

struct raon_parser_block { };

struct raon_parser_entry {
   char* field;
   enum raon_token_type value_type;
   union {
      char* string_val;
      int int_val;
      bool bool_val;
      struct raon_parser_block block_val;
   };
};

int main(void) {
   char* buffer = 0;
   long length;
   FILE* f = fopen("./example.raon", "rb");
   struct raon_lexer lex = raon_lexer_init_from_file(f);
   if (!lex.str) {
      perror("reading file failed");
      return 1;
   }

   puts(lex.str);

   size_t token_len = 0;
   struct raon_token_vec* tokens = raon_lexer_lex(&lex, &token_len);
   if (!tokens) {
      return 1;
   }

   // cleanup:
   raon_lexer_free(&lex);
   raon_token_vec_free(tokens);

   return 0;
}
