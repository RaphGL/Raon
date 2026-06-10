#ifndef RAON_STR_SLICE_H
#define RAON_STR_SLICE_H

#include <stddef.h>

struct raon_str_slice {
   char *ptr;
   size_t len;
};

// returns an allocated string. the string ought to be freed by the user.
char *raon_str_from_slice(struct raon_str_slice slice);
struct raon_str_slice raon_slice_from_str(char *str, size_t len);
char *raon_copy_slice_to_str(char *dest, struct raon_str_slice src, size_t size);

#endif
