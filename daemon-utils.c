#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>

#include "daemon-utils.h"

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
  if (! (pid > 0) ) pid = getpid();

  FILE * f = fopen(pidFile, "w");

  if (!f) return -1;

  fprintf(f, "%d", pid);

  fclose(f);
  return pid;
}

/* -------------------------------------------------------------------------- */

int
daemon_set_user( const char * user_name )
{
  struct passwd * pw = getpwnam( user_name );
  if (pw)
  {
    return setuid(pw->pw_uid);
  }
  else
    return -1;
}

/* -------------------------------------------------------------------------- */

pid_t
daemon_pid_check( const char * pidFile )
{
  pid_t result;

  if ( access( pidFile, F_OK) != -1)
  {
    result = daemon_pid_read( pidFile );

    if (kill(result, 0) == -1 && errno != EPERM)
      result = 0;
  }
  else
    result = 0;
  
  return result;
}

/* -------------------------------------------------------------------------- */

