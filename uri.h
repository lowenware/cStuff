#ifndef _STANDARD_URI_H_
#define _STANDARD_URI_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct
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
} uri_t;


bool
uri_parse(const char *in, int in_len, uri_t *out);

#endif
