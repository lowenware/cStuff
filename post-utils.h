#ifndef _CSTUFF_POST_UTILS_C_
#define _CSTUFF_POST_UTILS_C_

#include <stdbool.h>
#include <inttypes.h>
#include <cStuff/query-stream.h>

/* -------------------------------------------------------------------------- */

/* RETURN VALUES:
    1 - exit internal loop (has all data) *
    0 - success and continue
   -1 - error */
typedef int
(*post_query_on_pair_t)( query_stream_t query_stream, void * u_pointer );

/* -------------------------------------------------------------------------- */

struct post_query
{
  struct query_stream      query_stream;
  char                   * data;
  uint32_t                 data_size;
  uint32_t                 content_length;
};

typedef struct post_query * post_query_t;

/* -------------------------------------------------------------------------- */

int
post_query_init(post_query_t self);

/* -------------------------------------------------------------------------- */

void
post_query_release(post_query_t self);

/* -------------------------------------------------------------------------- */

int
post_query_feed_header(post_query_t self, const char * key, const char * value);

/* -------------------------------------------------------------------------- */

int
post_query_feed_data(post_query_t self, const char * data, int len);

/* -------------------------------------------------------------------------- */

int
post_query_read(post_query_t self, post_query_on_pair_t on_pair, void * u_ptr);

/* -------------------------------------------------------------------------- */

#endif
