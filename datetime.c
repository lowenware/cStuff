#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "datetime.h"


datetime_t *
datetime_now(datetime_t * dt)
{
  if (!dt)
    if ((dt = malloc(sizeof(datetime_t))) == NULL) return NULL;

  clock_gettime(CLOCK_REALTIME, (struct timespec *) dt);

  return dt;
}

bool
datetime_from_string(datetime_t * dt, const char *dtstr, 
                                      int len,
                                      const char * timezone)
{
  char dt_buf[26+1],
       * cur_tz;
  
  if (len != DATETIME_NTS)
  {
    if (len > 26) len = 26;
    strncpy(dt_buf, dtstr, len);
    dt_buf[len]=0;
    dtstr = dt_buf;
  }
  else
    len = strlen(dtstr);

  /* YYYY-MM-DD HH:MM:SS.000000 */
  if (len < 19 || dtstr[4] !='-' || dtstr[7] !='-' ||
     (dtstr[10] !=' ' && dtstr[10] !='T') || dtstr[13] !=':' || dtstr[16] !=':')
    return false;

  struct tm tm = {0,0,0,0,0,0,0,0,0};

  tm.tm_year = strtol(dtstr,      NULL, 10) - 1900;
  tm.tm_mon  = strtol(&dtstr[5],  NULL, 10) - 1;
  tm.tm_mday = strtol(&dtstr[8],  NULL, 10);
  tm.tm_hour = strtol(&dtstr[11], NULL, 10);
  tm.tm_min  = strtol(&dtstr[14], NULL, 10);
  tm.tm_sec  = strtol(&dtstr[17], NULL, 10);
  tm.tm_isdst = -1;

  /*
    printf(
      "%04d-%02d-%02d %02d:%02d:%02d\n",
      tm.tm_year+1900,
      tm.tm_mon,
      tm.tm_mday,
      tm.tm_hour,
      tm.tm_min,
      tm.tm_sec
    );
  */

  if (timezone)
  {
    cur_tz = getenv("TZ");
    setenv("TZ", timezone, 1);
  }
  else
    cur_tz = NULL;

  dt->sec = mktime(&tm);

  if (timezone)
  {
    if (cur_tz)
      setenv("TZ", cur_tz, 1);
    else
      putenv("TZ");
  }


  if (len > 20)
  {
    if (dtstr[19] != '.') return false;
    dt->n_sec = strtol(&dtstr[20], NULL, 10)*1000;
  }
  else
    dt->n_sec = 0;

  return true;
}

char *
datetime_to_string( datetime_t    *dt,
                    char          *dtstr,
                    datetime_tz_t  tz )
{
  if (!dtstr)
    dtstr = malloc(DATETIME_STRING_SIZE+1);

  char usec[20];

  struct tm * tm = (tz==DATETIME_LOCAL) ? localtime(&dt->sec):gmtime(&dt->sec);

  strftime(dtstr, DATETIME_STRING_SIZE+1, DATETIME_STRING_FORMAT, tm);
#ifdef DATETIME_MICROSECONDS
  sprintf(usec, ".%03ld", dt->n_sec / 1000000);
#else
  sprintf(usec, ".%06ld", dt->n_sec / 1000);
#endif
  strcat(dtstr, usec);

  return dtstr;
}


char *
datetime_format( datetime_t * dt,
                 const char * format,
                 const char * timezone,
                 char       * out,
                 int          o_size)
{
  if (!out) return NULL;

  char * cur_tz;
  struct tm * tm;
  
  if (timezone)
  {
    cur_tz = getenv("TZ");
    setenv("TZ", timezone, 1);
  }
  else
    cur_tz = NULL;

  tm = localtime(&dt->sec) ;
  strftime(out, o_size, format, tm);

  if (timezone)
  {
    if (cur_tz)
      setenv("TZ", cur_tz, 1);
    else
      putenv("TZ");
  }

  return out;
}


int64_t
datetime_compare(datetime_t * dt1, datetime_t * dt2)
{
  int64_t result;

  if (dt1->sec > dt2->sec)
  {
    result = dt1->sec - dt2->sec;
  }
  else if (dt2->sec > dt1->sec)
  {
    result = -1 * (dt2->sec - dt1->sec);
  }
  else
  {
    if (dt1->n_sec > dt2->n_sec)
      result = 1;
    else if (dt1->n_sec < dt2->n_sec)
      result = -1;
    else
      result = 0;
  }

  return result;
}
