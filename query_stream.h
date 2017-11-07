#ifndef _WEBSTUFF_QUERY_STREAM_H_
#define _WEBSTUFF_QUERY_STREAM_H_

#include <stdbool.h>

typedef struct
{
  const char * key;                    /* pointer to key */
  const char * value;                  /* pointer to value */
  int    key_len;                      /* key length */
  int    value_len;                    /* value length */
  char   sep;                          /* separator character */

} query_stream_t;

#define QUERY_STREAM_INIT( SP ) {NULL, NULL, -1, -1, SP}

int
query_stream_read(query_stream_t * q, const char * data, int size, bool is_full);

#endif
