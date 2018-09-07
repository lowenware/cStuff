#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "str-utils.h"
#include "post-utils.h"

/* -------------------------------------------------------------------------- */

int
post_query_init(post_query_t self)
{
  self->query_stream.sep = '&';
  return 0;
}

/* -------------------------------------------------------------------------- */

void
post_query_release(post_query_t self)
{
  if (self->data)
    free(self->data);
}

/* -------------------------------------------------------------------------- */

int
post_query_feed_header(post_query_t self, const char * key, const char * value)
{
  if ( strcmp(key, "content-length")==0 )
  {
    self->content_length = strtol(value, NULL, 10);
    return 0;
  }

  return 1;
}

/* -------------------------------------------------------------------------- */

int
post_query_feed_data(post_query_t self, const char * data, int len)
{
  char * p_data;

  if ( (p_data = str_ncat(self->data, data, len)) != NULL )
  {
    if (self->content_length < len)
      return -1;

    self->content_length -= len;
    self->data_size      += len;
    self->data            = p_data;

    query_stream_reset(&self->query_stream);
    /*
    memset(&self->query_stream, 0, sizeof(struct query_stream));
    q->sep = '&'
    */
    return 0;
  }
  else
    return -1;
}

/* -------------------------------------------------------------------------- */

int
post_query_read(post_query_t self, post_query_on_pair_t on_pair, void * u_ptr)
{
  query_stream_t q = &self->query_stream;

  int result = 1,
      res,         /* parser result */
      read   = 0,
      total  = self->data_size;

  char * data = self->data;

  result = 1;

  fprintf(stderr, ": DATA: %s, %u, %u\n", data, total, self->content_length);

  while (
    (res = query_stream_read(q, data, total, (self->content_length == 0))) > 0
  )
  {
    result = on_pair(q, u_ptr);
    fprintf(stderr, ": res=%d, result=%d\n", res, result);

    read = res;

    if (result != 0)
      break;
  }

  total -= read;

  if (total)
    memmove(data, &data[read], total);

  data[total] = 0;

  return result;
}

/* -------------------------------------------------------------------------- */
