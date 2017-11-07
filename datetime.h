#ifndef _STANDARD_DATETIME_H_
#define _STANDARD_DATETIME_H_

#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef DATETIME_STRING_FORMAT
#define DATETIME_STRING_FORMAT "%Y-%m-%d %H:%M:%S"
#endif

/* type --------------------------------------------------------------------- */

struct datetime
{
  time_t   sec;
  long     n_sec;
};

typedef struct datetime datetime_t;

typedef enum {
  DATETIME_UTC,
  DATETIME_LOCAL

} datetime_tz_t;

#define DATETIME_NTS -1
#define DATETIME_STRING_SIZE 26
#define DATETIME_NULL {0,0}

/* methods ------------------------------------------------------------------ */

datetime_t *
datetime_now(datetime_t * dt);

bool
datetime_from_string(datetime_t * dt, const char *dtstr, 
                                      int len,
                                      const char * timezone);

char *
datetime_to_string( datetime_t    *dt,
                    char          *dtstr, 
                    datetime_tz_t  tz );


char *
datetime_format( datetime_t * dt,
                 const char * format,
                 const char * timezone,
                 char       * out ,
                 int          size);

int64_t
datetime_compare(datetime_t * dt1, datetime_t * dt2);

#endif
