#ifndef _CSTUFF_DAEMON_UTILS_H_
#define _CSTUFF_DAEMON_UTILS_H_

#include <unistd.h>
#include <sys/types.h>

/* configuration macroses --------------------------------------------------- */

#ifndef CSTUFF_PID_BUFFER_SIZE
#define CSTUFF_PID_BUFFER_SIZE  16
#endif

/* -------------------------------------------------------------------------- */

pid_t
daemon_pid_read( const char * pidFile );

/* -------------------------------------------------------------------------- */

pid_t
daemon_pid_write( const char * pidFile, pid_t pid );

/* -------------------------------------------------------------------------- */

int
daemon_set_user( const char * user_name );

/* -------------------------------------------------------------------------- */

int
daemon_pid_check( const char * pidFile );

/* -------------------------------------------------------------------------- */

#endif
