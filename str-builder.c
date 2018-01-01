#include <stdlib.h>
#include <string.h>

#include "str-builder.h"

/* -------------------------------------------------------------------------- */

str_builder_t
str_builder_new( int size )
{
  str_builder_t self = calloc( 1, sizeof( struct str_builder ) );
  if (self)
  {
    if (size)
    {
      if ( !(self->c_str = calloc(size, sizeof(char))) )
      {
        free(self);
        self = NULL;
      }
      else
        self->size = size;
    }
  }

  return self;
}

/* -------------------------------------------------------------------------- */

void
str_builder_free( str_builder_t self, int free_data )
{
  if (self->c_str && free_data )
  {
    free( self->c_str );
  }
  free( self );
}

/* -------------------------------------------------------------------------- */

int
str_builder_append( str_builder_t self, const char * c_str )
{
  return str_builder_append_chars( self, c_str, strlen(c_str) );
}

/* -------------------------------------------------------------------------- */

int
str_builder_append_chars( str_builder_t self, const char * chars, int size )
{
  int result;

  char  * tmp;

  if (size)
  {
    result = self->length + size;
    if ( result > self->size )
    {
      if ( !(tmp = realloc( self->c_str, result + 1 )) )
        return -1;

      self->c_str = tmp;

      if (!self->size)
        *self->c_str = 0;

      self->size = result;
    }
    strncat( self->c_str, chars, size );
    self->length = result;
  }
  else
    result = 0;

  return result;
}

/* -------------------------------------------------------------------------- */
