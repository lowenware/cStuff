#ifndef _CSTUFF_URI_H_
#define _CSTUFF_URI_H_

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_URI_WITH_PARSE

#include <stdint.h>
#include <stdbool.h>

struct uri
{
  int16_t protocol_len;
  int16_t user;
  int16_t user_len;
  int16_t password;
  int16_t password_len;
  int16_t address;
  int16_t address_len;
  int16_t port;
  int16_t port_len;
  int16_t path;
  int16_t path_len;
  int16_t query;
  int16_t query_len;
};

typedef struct uri * uri_t;


bool
uri_parse(const char *in, int in_len, uri_t out);

#endif 

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_URI_WITH_DECODE

/* decode uri stored in @data to normal text */
void
uri_decode(char *data);

#endif

/* -------------------------------------------------------------------------- */

#endif
