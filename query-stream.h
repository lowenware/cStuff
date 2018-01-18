#ifndef _CSTUFF_QUERY_STREAM_H_
#define _CSTUFF_QUERY_STREAM_H_

#include <stdbool.h>

/* -------------------------------------------------------------------------- */

struct query_stream
{
  const char * key;                          /* pointer to key */
  const char * value;                        /* pointer to value */
  int          key_len;                      /* key length */
  int          value_len;                    /* value length */
  char         sep;                          /* separator character */

};

typedef struct query_stream * query_stream_t;

#define QUERY_STREAM_INIT( SP ) {NULL, NULL, -1, -1, SP}

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_QUERY_STREAM_WITH_CONSTRUCTOR

query_stream_t
query_stream_new( char separator );

#define query_stream_free( self ) free(self)

#endif

/* -------------------------------------------------------------------------- */

int
query_stream_read(query_stream_t q, const char * data, int size, bool is_full);

/* -------------------------------------------------------------------------- */

#endif
