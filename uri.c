#include <stdlib.h>
#include "utils.h"


#ifdef WEBSTUFF_WITH_DECODE_URI

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

