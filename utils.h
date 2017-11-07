#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

#include <stdint.h>
#include <stdarg.h>

#include <arpa/inet.h>


/* String Commons ----------------------------------------------------------- */

char *
u_strdup(const char * src);


char *
u_strndup(const char * src, uint32_t s);

char *
u_strset(char * init, const char * src);

char *
u_sprintf(const char * format,  ...);

char *
u_vsprintf(const char * format,  va_list vl);


/* for source will be called u_free automatically */
char *
u_strcat(char * source, const char * target);

/* for source will be called u_free automatically */
char *
u_strncat(char * source, const char * target, uint32_t length);

/* networking --------------------------------------------------------------- */

void
u_inetaddr_to_str(struct sockaddr_in * sa, char * sa_str);


/* Timer -------------------------------------------------------------------- */

/*
void
u_stopwatch_start(stopwatch_t * sw);


void
u_stopwatch_stop(stopwatch_t * sw);


uint32_t
u_stopwatch_get_elapsed(stopwatch_t * sw);
*/

void
u_sleep(uint32_t usec);

/* timestamp ---------------------------------------------------------------- */

/*
WsTimestamp *
u_now(WsTimestamp * ts, WsTimezone tz);


void
u_timestamp_set_from_string(WsTimestamp * ts, char * string);


void
u_timestamp_set_string(WsTimestamp * ts, WsString * string);


void
u_timestamp_add_seconds(WsTimestamp * ts, WsInt seconds);


WsTimestamp *
u_timestamp_from_ascii(WsTimestamp * ts, char * string);

*/
/* Free --------------------------------------------------------------------- */

void *
u_malloc(uint32_t size);

void *
u_calloc(uint32_t length, uint32_t size);

void
u_free(void * ptr);

void *
u_nfree(void * ptr);

#endif
