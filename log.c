#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"
#include "str-utils.h"

#ifdef CSTUFF_LOG_WITH_STDLOG

log_t stdlog = NULL;

#endif

const char   cAsciiLine[] = "+-----------------------------------------------------------------------------+\n";

struct _log_t
{
/*  pthread_mutex_t   lock; */
  char            * file;
  log_level_t       level;
};


/* -------------------------------------------------------------------------- */

log_t
log_new()
{
  log_t self  = calloc(1, sizeof(struct _log_t));
  self->level = 0;

/*  pthread_mutex_init(&self->lock, NULL); */

  return self;
}

void
log_free(log_t self)
{
  if (self)
  {
    if (self->file)
    {
      log_printf(
        self, LOG_STATE, 
        "End logging session -----------------------------"
      );
      free(self->file);
    }
    /* pthread_mutex_destroy(&self->lock); */
    free(self);
  }
}

bool
log_set_level(log_t self, uint32_t level)
{
  if (level != self->level)
  {
    self->level = level;
    return true;
  }
  else
    return false;

}

uint32_t
log_get_level(log_t self)
{
  return self->level;
}

bool
log_set_file(log_t self, const char * file)
{

  if ((!self->file || !file || strcmp(self->file, file)))
  {
    /* pthread_mutex_lock(&self->lock); */
    self->file = str_copy(file);
    /* pthread_mutex_unlock(&self->lock); */
    if (self->file)
    {
      log_printf(
        self, LOG_STATE, 
        "Begin logging session (level: %d) ---------------", 
        self->level
      );
    }
    return true;
  }
  else
    return false;

}

const char *
log_get_file(log_t self)
{
  return self->file;
}


void
log_vprintf(log_t self, log_level_t level, const char * format, va_list args)
{
  FILE * f;
  char * label;

  time_t    rawtime;
  char      strtime[32+1];
  struct tm timeinfo;



  /* pthread_mutex_lock(&self->lock); */

  if (self->level & level)
  {
    time (&rawtime);
    localtime_r (&rawtime, &timeinfo);
    strftime (strtime, 32, "%c", &timeinfo);

    switch (level)
    {
      case LOG_ERROR : label = "!!"; break;
      case LOG_ALERT : label = "~!"; break;
      case LOG_DEBUG : label = "**"; break;
      default       : label = "--";
    }

    f = (self->file) ? fopen(self->file, "a+") : stdout;

    if (f == NULL) f = stdout;

    fprintf(f, "%s %s ", strtime, label);
    vfprintf (f, format, args);
    if (format[strlen(format)-1] != '\n') fputc('\n', f);

    if (f != stdout) fclose(f);
  }

  /* pthread_mutex_unlock(&self->lock); */
}

void
log_printf(log_t self, log_level_t level, const  char * format, ... )
{
  va_list arg;

  va_start(arg, format);
  log_vprintf(self, level, format, arg);
  va_end(arg);
}


