#include "raon.h"
#include "../vendor/ht.h"
#include <stdio.h>

typedef Ht(const char *, struct raon_value) Raon_HashTable;

#define ALLOCATOR_CAST(to, from)                                                                   \
   ((to) {                                                                                         \
       .alloc = from.alloc,                                                                        \
       .free = from.free,                                                                          \
   })

Raon_HashTable *raon_hashtable_from_entries(
    Ht_Allocator allocator, struct vector_of_raon_entry *entries) {
   if (vec_len_raon_entry(entries) == 0) {
      return NULL;
   }

   for (size_t i = 0; i < vec_len_raon_entry(entries); i++) {
      struct raon_entry entry;
      vec_get_raon_entry(entries, i, &entry);
   }
}

Raon_HashTable *raon_load_from_file(Ht_Allocator allocator, const char *filepath) {
   FILE *fd = fopen(filepath, "r");
   if (!fd) {
      return NULL;
   }

   bool error_happened = false;

   fseek(fd, 0, SEEK_END);
   size_t file_size = ftell(fd);
   rewind(fd);

   char *file_contents = allocator.alloc(file_size + 1);
   if (!file_contents) {
      error_happened = true;
      goto cleanup;
   }
   fread(file_contents, 1, file_size, fd);
   if (ferror(fd)) {
      error_happened = true;
      goto cleanup;
   }
   file_contents[file_size] = '\0';

   struct vector_of_raon_entry *entries
       = raon_parse(ALLOCATOR_CAST(struct vec_allocator, allocator), file_contents, file_size + 1);
   if (!entries) {
      error_happened = true;
      goto cleanup;
   }

   Raon_HashTable *ht = raon_hashtable_from_entries(allocator, entries);
   if (!ht) {
      error_happened = true;
      goto cleanup;
   }
   return ht;

cleanup:
   fclose(fd);
   free(file_contents);
   if (error_happened) {
      return NULL;
   }
   return ht;
}

Raon_HashTable *raon_load_from_buffer(Ht_Allocator allocator, char *buffer) {
   struct vector_of_raon_entry *entries = raon_parse(ALLOCATOR_CAST(struct vec_allocator, allocator), buffer, strlen(buffer));
   if (!entries) {
      return NULL;
   }

   return raon_hashtable_from_entries(allocator, entries);
}
