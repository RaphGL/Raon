#ifndef VEC_ITEM_TYPE
   #error VEC_ITEM_TYPE was not declared
#endif

#ifndef VEC_SUFFIX
   #define VEC_SUFFIX VEC_ITEM_TYPE
#endif

#define __G(name, type) name##_##type
#define _G(name, type) __G(name, type)
#define G(name) _G(name, VEC_SUFFIX)

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef VEC_IMPLEMENTATION

typedef struct G(vector_of) {
   size_t capacity;
   size_t len;
   VEC_ITEM_TYPE *vec;
} G(Vector_of);

G(Vector_of) * G(vec_new)(void);
bool G(vec_fit)(G(Vector_of) *vec);
bool G(vec_push)(G(Vector_of) *vec, VEC_ITEM_TYPE item);
size_t G(vec_len)(const G(Vector_of) * vec);
size_t G(vec_capacity)(const G(Vector_of) * vec);
bool G(vec_pop)(G(Vector_of) *vec, VEC_ITEM_TYPE *dest);
void G(vec_free)(G(Vector_of) *vec);
bool G(vec_get)(const G(Vector_of) * vec, const size_t index, VEC_ITEM_TYPE *dest);
void G(vec_printf)(const char *fmt, const G(Vector_of) * vec);
bool G(vec_insert)(G(Vector_of) * vec, const size_t index, VEC_ITEM_TYPE item);
bool G(vec_remove)(G(Vector_of) *vec, const size_t index, VEC_ITEM_TYPE *dest);
bool G(vec_is_empty)(const G(Vector_of) * vec);
bool G(vec_append)(G(Vector_of) *dest, const G(Vector_of) * src);
G(Vector_of) * G(vec_from)(VEC_ITEM_TYPE *const arr, size_t length, size_t item_size);

#else

// Initializes a new vector with items of sizeof(T)
G(Vector_of) * G(vec_new)(void) {
   G(Vector_of) *vector = malloc(sizeof(*vector));
   if (!vector) {
      return NULL;
   }

   *vector = (G(Vector_of)) {
      .capacity = 0,
      .len = 0,
      .vec = NULL,
   };

   return vector;
}

// Resizes vector to fit length
bool G(vec_fit)(G(Vector_of) *vec) {
   const size_t power = ceilf(log2f(vec->len + 1));
   const size_t new_capacity = sizeof(vec->vec[0]) * powf(2, power);

   if (new_capacity < vec->capacity || new_capacity > vec->capacity) {
      void *tmp = realloc(vec->vec, new_capacity);
      if (!tmp) {
         return false;
      }
      vec->vec = tmp;
      vec->capacity = new_capacity;
   }
   return true;
}

// Pushes a value to vector
bool G(vec_push)(G(Vector_of) *vec, VEC_ITEM_TYPE item) {
   if (vec->len == 0 && vec->capacity == 0) {
      vec->vec = malloc(sizeof(vec->vec[0]));
      if (!vec->vec) {
         return false;
      }

      vec->vec[0] = item;
      vec->len = 1;
      vec->capacity = sizeof(vec->vec[0]);
      return true;
   }

   G(vec_fit)(vec);
   vec->vec[vec->len] = item;
   vec->len++;
   return true;
}

// Returns how many items are in the vector
size_t G(vec_len)(const G(Vector_of) * vec) { return vec->len; }

// Returns the total amount of items that can currently be stored
size_t G(vec_capacity)(const G(Vector_of) * vec) { return vec->capacity; }

// Removes an item from the end of the vector and assigns it to dest
bool G(vec_pop)(G(Vector_of) *vec, VEC_ITEM_TYPE *dest) {
   if (!vec->vec || vec->len <= 0 || vec->capacity <= 0) {
      return false;
   }

   vec->len--;
   if (dest) {
      *dest = vec->vec[vec->len];
   }
   G(vec_fit)(vec);
   return true;
}

// Clears out all the memory used by the vector
void G(vec_free)(G(Vector_of) *vec) {
   free(vec->vec);
   vec->vec = NULL;
   free(vec);
}

// Returns item at index
bool G(vec_get)(const G(Vector_of) * vec, const size_t index, VEC_ITEM_TYPE *dest) {
   if (!vec->vec || index >= vec->len || vec->capacity == 0) {
      return false;
   }

   *dest = vec->vec[index];
   return true;
}

// Prints the vector with the specified format specifier
void G(vec_printf)(const char *fmt, const G(Vector_of) * vec) {
   printf("[");
   for (size_t i = 0; i < G(vec_len)(vec); i++) {
      printf(" ");
      printf(fmt, vec->vec[i]);
   }
   printf(" ]\n");
}

// Inserts a new item at index
bool G(vec_insert)(G(Vector_of) * vec, const size_t index, VEC_ITEM_TYPE item) {
   size_t len = vec->len - 1; // length before growing
   if (index > len) {
      return false;
   }

   G(vec_push)(vec, vec->vec[len]);

   for (size_t i = len; i > index; i--) {
      vec->vec[i - 1] = vec->vec[i];
   }

   vec->vec[index] = item;
   return true;
}

// Removes an item at index
bool G(vec_remove)(G(Vector_of) *vec, const size_t index, VEC_ITEM_TYPE *dest) {
   size_t len = vec->len - 1;
   if (index > len) {
      return false;
   }

   if (dest) {
      *dest = vec->vec[index];
   }

   for (size_t i = index; i < vec->len; i++) {
      if (i + 1 < vec->len) {
         vec->vec[i] = vec->vec[i + 1];
      }
   }

   return G(vec_pop)(vec, NULL);
}

bool G(vec_is_empty)(const G(Vector_of) * vec) {
   if (vec->len == 0 || vec->capacity == 0 || !vec->vec) {
      return true;
   }

   return false;
}

// Appends vector from src to dest
bool G(vec_append)(G(Vector_of) *dest, const G(Vector_of) * src) {
   for (size_t i = 0; i < src->len; i++) {
      if (!G(vec_push)(dest, src->vec[i])) {
         return false;
      }
   }

   return true;
}

G(Vector_of) * G(vec_from)(VEC_ITEM_TYPE *const arr, size_t length, size_t item_size) {
   G(Vector_of) *vec = G(vec_new)();
   for (size_t i = 0; i < length; i++) {
      if (!G(vec_push)(vec, arr[i])) {
         G(vec_free)(vec);
         return NULL;
      }
   }
   return vec;
}

#endif
#undef VEC_ITEM_TYPE
#undef VEC_SUFFIX
