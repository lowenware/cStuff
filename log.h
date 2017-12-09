#ifndef _CSTUFF_LOG_H_
#define _CSTUFF_LOG_H_

/* Class log_t -------------------------------------------------------------- */

#include <stdarg.h>

/* Log Levels --------------------------------------------------------------- */

#define LOG_ALL  ( LOG_LEVEL_ERROR | LOG_LEVEL_ALERT | \
                   LOG_LEVEL_STATE | LOG_LEVEL_DEBUG )

typedef enum
{
  LOG_LEVEL_DISABLED = 0,
  LOG_LEVEL_ERROR    = 1,
  LOG_LEVEL_ALERT    = 2,
  LOG_LEVEL_STATE    = 4,
  LOG_LEVEL_DEBUG    = 8

} log_level_t;

/* Log Structure ------------------------------------------------------------ */

struct log
{
  const char * file;
  int          level;
};

typedef struct log * log_t;

/* Methods ------------------------------------------------------------------ */

/* creates new object of log_t
 * @return new object of log_t
 * */
log_t
log_new();

/* -------------------------------------------------------------------------- */

/* clear resources used by log_t
 * @param self log_t object
 * */
void
log_free(log_t self);

/* -------------------------------------------------------------------------- */

void
log_set_level(log_t self, const char * level);

/* -------------------------------------------------------------------------- */

/* save formated message to log
 * @param self log_t object
 * @param level defines level of logging message
 * @param format formated null-terminated string to be logged
 * @param arg standard va_list of passed arguments
 * */
int
log_printf(log_t self, log_level_t level, const char * format, ... );

/* -------------------------------------------------------------------------- */

int
log_vprintf(log_t self, log_level_t level, const char * format, va_list arg);

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LOG_WITH_STDLOG

#define log_error(...) log_printf(&stdlog, LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_alert(...) log_printf(&stdlog, LOG_LEVEL_ALERT, __VA_ARGS__)
#define log_state(...) log_printf(&stdlog, LOG_LEVEL_STATE, __VA_ARGS__)
#define log_debug(...) log_printf(&stdlog, LOG_LEVEL_DEBUG, __VA_ARGS__)

extern struct log stdlog;

#endif

/* -------------------------------------------------------------------------- */
#endif
