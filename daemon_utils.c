#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "daemon_utils.h"

static char pidbuf[ CSTUFF_PID_BUFFER_SIZE ];

/* -------------------------------------------------------------------------- */

pid_t
daemon_pid_read( const char * pidFile )
{
  int     l;
  pid_t   pid;
  FILE  * f = fopen(pidFile, "r");

  if (!f) return -1;
  
  memset(pidbuf, 0, sizeof(pidbuf));

  l = fread(pidbuf, 1, sizeof(pidbuf), f);
  pid = (l>0) ? strtol(pidbuf, NULL, 10) : 0;

  fclose(f);
  return pid;
}

/* -------------------------------------------------------------------------- */

pid_t
daemon_pid_write( const char * pidFile, pid_t pid )
{
  if (! (pid_t > 0) ) pid = getpid();

  FILE * f = fopen(pidFile, "w");

  if (!f) return -1;

  fprintf(f, "%d", pid);

  fclose(f);
  return pid;
}

/* -------------------------------------------------------------------------- */
