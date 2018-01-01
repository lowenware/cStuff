#ifndef _CSTUFF_STR_BUILDER_H_
#define _CSTUFF_STR_BUILDER_H_

/* -------------------------------------------------------------------------- */

struct str_builder {
  char * c_str;
  int    size;
  int    length;
};

typedef struct str_builder * str_builder_t;

/* -------------------------------------------------------------------------- */

str_builder_t
str_builder_new( int size );

/* -------------------------------------------------------------------------- */

void
str_builder_free( str_builder_t self, int free_data );

/* -------------------------------------------------------------------------- */

int
str_builder_append( str_builder_t self, const char * c_str );

/* -------------------------------------------------------------------------- */

int
str_builder_append_chars( str_builder_t self, const char * chars, int size );

/* -------------------------------------------------------------------------- */

#endif
