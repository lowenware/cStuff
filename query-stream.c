#include <stdlib.h>
#include <string.h>

#include "str-utils.h"
#include "uri.h"
#include "query-stream.h"

/* -------------------------------------------------------------------------- */


#ifdef CSTUFF_QUERY_STREAM_WITH_CONSTRUCTOR

query_stream_t
query_stream_new( char separator )
{
  query_stream_t self;

  if ( (self=calloc(1, sizeof(struct query_stream))) !=NULL )
  {
    self->key_len   = -1;
    self->value_len = -1;
    self->sep       = separator;
  }

  return self;
}

#endif

/* -------------------------------------------------------------------------- */

void
query_stream_reset(query_stream_t q)
{
    char separator = q->sep;
    memset(q, 0, sizeof(struct query_stream));
    q->sep = separator;
}

/* -------------------------------------------------------------------------- */

int
query_stream_read(query_stream_t q, const char * data, int size, bool is_full)
{
  int i = (q->value) ? (int) (q->value - data) + q->value_len + 1 : 0;

  if (i < size)
  {
    q->key       = &data[i];
    q->value     = NULL;
    q->key_len   = 0;
    q->value_len = 0;

    while ( i < size)
    {
      if (data[i] == '=' && !q->value)
          q->value = &data[i+1];
      else if (data[i] == q->sep)
        break;
      else if (q->value)
        q->value_len++;
      else if (data[i] == ' ') /* there MUST be no spaces in key name */
      {
        q->key     = &data[i+1];
        q->key_len = 0;
      }
      else
        q->key_len++;

      i++;
    }

    if (!q->value || (i == size && !is_full) ) i = 0;
  }
  else
    i = 0;

  return i;
}

/* -------------------------------------------------------------------------- */

int
query_stream_copy_value(query_stream_t q, const char * key, char ** value)
{
  if (strncmp(q->key, key, q->key_len)==0)
  {
    if ( !(*value) )
    {
      if ( !(*value = str_ncopy(q->value, q->value_len)) )
        return -1;

      uri_decode(*value);
    }

    return 0;
  }

  return 1;
}

/* -------------------------------------------------------------------------- */
