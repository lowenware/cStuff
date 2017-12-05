#include <stdlib.h>

#include "query-stream.h"

int
query_stream_read(query_stream_t * q, const char * data, int size, bool is_full)
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

