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
  raon_token_type_string,
  raon_token_type_bool,
  raon_token_type_int,
  raon_token_type_field,
  raon_token_type_equal,
  raon_token_type_block_open,
  raon_token_type_block_close,
};

struct raon_token {
  size_t x, y;
  enum raon_token_type type;

  union {
    char *string_val;
    int int_val;
    bool bool_val;
  };
};

struct raon_lexer {
  size_t curr_idx;
  size_t curr_x, curr_y;
  char *str;
  size_t str_len;
};

struct raon_lexer raon_lexer_init_from_str(char *str) {
  const size_t str_len = strlen(str);
  char *str_copy = malloc(str_len * sizeof(char));
  strncpy(str_copy, str, str_len);
  return (struct raon_lexer){
      .str = str_copy,
      .str_len = str_len,
  };
}

// initialization failed if str field is NULL
struct raon_lexer raon_lexer_init_from_file(FILE *file) {
  if (!file) {
    return (struct raon_lexer){
        .str = NULL,
    };
  }

  fseek(file, 0, SEEK_END);
  size_t len = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *str = raon_malloc(len);
  if (str) {
    fread(str, 1, len, file);
  }
  fclose(file);

  return (struct raon_lexer){
      .str = str,
  };
}

void raon_lexer_free(struct raon_lexer *lex) {
  raon_free(lex->str);
  lex->str = NULL;
}

static char raon_lexer_peek(struct raon_lexer *lex) {
  size_t next_tok = lex->curr_idx + 1;
  if (next_tok >= lex->str_len) {
    return '\0';
  }
  return lex->str[next_tok];
}

static void raon_lexer_eat(struct raon_lexer *lex) {
  size_t next_idx = lex->curr_idx + 1;
  if (next_idx >= lex->str_len) {
    return;
  }
  char next_tok = lex->str[next_idx];
  if (next_tok == '\n') {
    ++lex->curr_y;
    lex->curr_x = 0;
  } else {
    ++lex->curr_x;
  }
}

void raon_lexer_lex(struct raon_lexer *lex) {
  for (;;) {
    if (isspace(raon_lexer_peek(lex))) {
      continue;
    }

    // TODO: lex text here
  }
}

int main(void) {
  char *buffer = 0;
  long length;
  FILE *f = fopen("./config.raon", "rb");
  struct raon_lexer lex = raon_lexer_init_from_file(f);
  if (!lex.str) {
    perror("reading file failed");
    return 1;
  }

  puts(lex.str);

  return 0;
}
