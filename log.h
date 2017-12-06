#ifndef _STANDARD_LOG_H_
#define _STANDARD_LOG_H_

/* Class log_t -------------------------------------------------------------- */

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

/* Log Levels --------------------------------------------------------------- */

#define LOG_ALL  (LOG_ERROR | LOG_ALERT | LOG_STATE | LOG_DEBUG)

typedef enum {
    LOG_DISABLED = 0,
    LOG_ERROR    = 1,
    LOG_ALERT    = 2,
    LOG_STATE    = 4,
    LOG_DEBUG    = 8

} log_level_t;

/* Log Structure ------------------------------------------------------------ */

typedef struct _log_t * log_t;

/* Methods ------------------------------------------------------------------ */

/* creates new object of log_t
 * @return new object of log_t
 * */
log_t
log_new();


/* clear resources used by log_t
 * @param self log_t object
 * */
void
log_free(log_t self);

bool
log_set_level(log_t self, uint32_t level);

uint32_t
log_get_level(log_t self);

bool
log_set_file(log_t self, const char * file);

const char *
log_get_file(log_t self);

/* save formated message to log
 * @param self log_t object
 * @param level defines level of logging message
 * @param message formated null-terminated string to be logged
 * @param arg standard va_list of passed arguments
 * */

void
log_printf(log_t self, log_level_t level, const char * format, ... );

void
log_vprintf(log_t self, log_level_t level, const char * format, va_list arg);

#ifdef CSTUFF_LOG_WITH_STDLOG

#define log_error(...) log_printf(stdlog, LOG_ERROR, __VA_ARGS__)
#define log_alert(...) log_printf(stdlog, LOG_ALERT, __VA_ARGS__)
#define log_state(...) log_printf(stdlog, LOG_STATE, __VA_ARGS__)
#define log_debug(...) log_printf(stdlog, LOG_DEBUG, __VA_ARGS__)

extern log_t stdlog;

#endif

#endif
