#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"
#include "str-utils.h"

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LOG_WITH_STDLOG

struct log stdlog = {NULL, 0};

#endif

/* -------------------------------------------------------------------------- */

log_t
log_new()
{
  log_t self  = calloc(1, sizeof(struct log));

  return self;
}

/* -------------------------------------------------------------------------- */

void
log_free(log_t self)
{
  if (!self) return;

  free(self);
}

/* -------------------------------------------------------------------------- */

void
log_set_level(log_t self, const char * level)
{
  self->level = 0;

  if (level)
  {
    if (strcmp(level, "all") != 0)
    {
      if (strstr(level, "state")) self->level |= LOG_LEVEL_STATE; else
      if (strstr(level, "alert")) self->level |= LOG_LEVEL_ALERT; else
      if (strstr(level, "error")) self->level |= LOG_LEVEL_ERROR; else
      if (strstr(level, "debug")) self->level |= LOG_LEVEL_DEBUG;
    }
    else
      self->level = LOG_ALL;
  }
}

/* -------------------------------------------------------------------------- */

int
log_vprintf(log_t self, log_level_t level, const char * format, va_list args)
{
  int    result = 0;
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
      case LOG_LEVEL_ERROR : label = "!!"; break;
      case LOG_LEVEL_ALERT : label = "~!"; break;
      case LOG_LEVEL_DEBUG : label = "**"; break;
      default              : label = "--";
    }

    f = (self->file) ? fopen(self->file, "a+") : stdout;

    if (f == NULL) f = stdout;

    result += fprintf(f, "%s %s ", strtime, label);
    result += vfprintf (f, format, args);
    if (format[strlen(format)-1] != '\n')
    {
      fputc('\n', f);
      result++;
    }

    if (f != stdout) fclose(f);
  }

  return result;
}

int
log_printf(log_t self, log_level_t level, const  char * format, ... )
{
  int result;
  va_list arg;

  va_start(arg, format);
  result = log_vprintf(self, level, format, arg);
  va_end(arg);

  return result;
}


