#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

#include "sock_utils.h"

int
sock_select(int sd, uint32_t wait_usec, int mode)
{
  fd_set  fs,
         *rfs = NULL,
         *wfs = NULL;

  struct timeval tv;

  tv.tv_sec  = 0;
  tv.tv_usec = wait_usec;

  if (sd > -1)
  {
    FD_ZERO (&fs);
    FD_SET(sd, &fs);

    if (mode == SOCK_READ) rfs = &fs; else wfs = &fs;
  }

  return select(sd+1, rfs, wfs, NULL, &tv);

}

bool
sock_set_address(struct sockaddr_in * sa, const char * address, int port)
{
  int               rs;
  struct addrinfo * ai;

  memset(sa, 0, sizeof( struct sockaddr_in ));
  sa->sin_family = AF_INET;

  rs = getaddrinfo(address, NULL, NULL, &ai);

  if(rs != 0)
  {
    return false;
  }

  sa->sin_addr.s_addr=((struct sockaddr_in*)(ai->ai_addr))->sin_addr.s_addr;
  sa->sin_port       =htons(port);

  freeaddrinfo(ai);

  return true;

}
