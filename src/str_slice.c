#include "str_slice.h"
#include <stdlib.h>
#include <string.h>

// returns a zero value if the slice cannot contain the len specified
struct raon_str_slice raon_slice_from_str(char *str, size_t len) {
   if (len > strlen(str)) {
      return (struct raon_str_slice) { 0 };
   }
   return (struct raon_str_slice) {
      .ptr = str,
      .len = len,
   };
}

// returns an allocated string. the string ought to be freed by the user.
char *raon_str_from_slice(struct raon_str_slice slice) {
   size_t cstr_len = slice.len + 1;
   char *str = malloc(cstr_len);
   if (!str) {
      return NULL;
   }
   strncpy(str, slice.ptr, slice.len);
   str[slice.len] = '\0';
   return str;
}

char *raon_copy_slice_to_str(char *dest, struct raon_str_slice src, size_t size) {
   size_t cstr_len = src.len + 1;
   if (size < cstr_len) {
      return NULL;
   }
   strncpy(dest, src.ptr, src.len);
   dest[src.len] = '\0';
   return dest;
}
