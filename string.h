/* string.h
 *
 * string builder class
 * */

#ifndef _CSTUFF_STRING_H_
#define _CSTUFF_STRING_H_

#include <stdint.h>

/* type --------------------------------------------------------------------- */

typedef char * string_t;

#define string_get_size(x) ( *((int *)(x-8)) )
#define string_get_length(x) ( *((int *)(x-4)) )

/* -------------------------------------------------------------------------- */

string_t
string_new( const char * data, int length );

/* -------------------------------------------------------------------------- */

int
string_set( string_t self, const char * data, int length );

/* -------------------------------------------------------------------------- */

void
string_free( string_t self );


/* -------------------------------------------------------------------------- */

#endif
