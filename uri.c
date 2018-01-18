#include <stdlib.h>
#include <string.h>
#include "uri.h"

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_URI_WITH_PARSE

#define URI_PROTOCOL    0
#define URI_USER        1
#define URI_PASSWORD    2
#define URI_ADDRESS     3
#define URI_PORT        4
#define URI_PATH        5
#define URI_QUERY       6

bool
uri_parse(const char *in, int in_len, uri_t out)
{
  int step = URI_PROTOCOL,
      i,
      l=0;

  memset(out, 0, sizeof(struct uri));

  for( i=0; i< in_len; i++)
  {
    switch(in[i])
    {
      case ':':
        switch (step)
        {
          case URI_PROTOCOL:
            if (in_len-i < 3 || strncmp(&in[i], "://", 3)!=0)
              return false;

            out->protocol_len = l;
            step = URI_USER;
            i+=2;
            out->user = i+1;
            break;
          
          case URI_USER:
            out->user_len = l;
            out->password = i+1;
            step = URI_PASSWORD;
            break;
          
          case URI_ADDRESS:
            out->address_len = l;
            out->port = i+1;
            step = URI_PORT;
            break;
          
          default:
            return false;
        }
        break;

      case '@':
        switch (step)
        {
          case URI_USER:
            out->user_len = l;
            out->password = -1;
            out->password_len = 0;
            out->address = i+1;
            step = URI_ADDRESS;
            break;
          case URI_PASSWORD:
            out->password_len = l;
            out->address = i+1;
            step = URI_ADDRESS;
            break;
          case URI_PROTOCOL:
          case URI_ADDRESS:
          case URI_PORT:
          case URI_PATH:
            return false;
          default:
            l++;
        }
        break;

      case '?':
        switch (step)
        {
          case URI_PATH:
            out->path_len = l;
            out->query = i+1;
            out->query_len = in_len -i -1;
            return true;
            
          default:
            return false;
        }
        
      case '/':
        switch(step)
        {
          case URI_PROTOCOL:
            return false;
          
          case URI_PATH:
          case URI_QUERY:
            l++;
            continue;

          case URI_USER:
            out->address = out->user;
            out->address_len = l;
            out->user = -1;
            out->password = -1;
            out->port = -1;
            break;
          case URI_PASSWORD:
            out->address = out->user;
            out->address_len = out->user_len;
            out->port = out->password;
            out->port_len = l;
            out->user = -1;
            out->user_len = 0;
            out->password = -1;
            out->password_len = 0;
            break;

          case URI_ADDRESS:
            out->address_len = l;
            out->port = -1;
            break;

          case URI_PORT:
            out->port_len = l;
            break;
        }
        step = URI_PATH;
        out->path=i;
        l=1;
        continue;

      default:
        l++;
        continue;
    }
    l = 0;
  }
  return true;
}

#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_URI_WITH_DECODE

void
uri_decode(char *data)
{
  char  *p      = data,
         hex[3] = {0,0,0};

  do
  {
    switch(*data)
    {
      case '%':
        hex[0]=*(++data);
        hex[1]=*(++data);
        *p++ = (char)strtol(hex, NULL, 16);
        break;
      case '+':
        *p++=' ';
        break;
      default:
        *p++=*data;
    }
  } while(*data++!=0);
}

#endif

/* -------------------------------------------------------------------------- */
