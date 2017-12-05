#ifndef _STANDARD_SOCK_UTILS_H_
#define _STANDARD_SOCK_UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>

#define SOCK_READ  1
#define SOCK_WRITE 2

int
sock_select(int sd, uint32_t wait_usec, int mode);

#define sock_has_input(sd, wait_usec) sock_select(sd, wait_usec, 1)
#define sock_can_write(sd, wait_usec) sock_select(sd, wait_usec, 2)
#define u_sleep(x) sock_select(-1, x*1000, 1);


bool
sock_set_address(struct sockaddr_in * sa, const char * address, int port);

#endif
