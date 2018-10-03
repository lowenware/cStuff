/* str_utils.c - code file of the C string module
 * Copyright (c) 2017 Löwenware Ltd (https://lowenware.com)
 *
 * REPOSITORY:
 *   git://lowenware.com:standard.git
 * MAINTAINER:
 *   Ilja Kartaschoff <ik@lowenware.com>
 *
 * LICENSE and DISCLAIMER:
 *   All code stored in standard.git repository is designed to solve
 *   very common and widely meet development tasks. We are not about to patent
 *   wheels here, so all code you can find in this repository is FREE:
 *   you can use, redistribute and/or modify it without any limits or
 *   restrictions.
 *
 *   All code described above is distributed in hope to be useful for somebody
 *   else WITHOUT ANY WARRANTY, without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *   In case of questions or suggestions, feel free to contact maintainer.
 *
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str-utils.h"

#ifndef CSTUFF_TIMESTAMP_FORMAT
#define CSTUFF_TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S"
#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_SET

char *
str_set(char * dst, const char * src)
{
  int len = (src) ? strlen(src) : 0;

  if (!dst || len > strlen(dst))
  {
    char * r;

    if ( (r = realloc( dst, len + 1 )) != NULL )
    {
      dst = r;
    }
    else
      return r;
  }

  if (len)
    strncpy(dst, src, len);

  dst[len]=0;

  return dst;
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_COPY

char *
str_copy(const char * src)
{
    if (!src) return NULL;

    uint32_t s = strlen(src);
    char * result = malloc((s+1)*sizeof(char));

    strcpy(result, src);

    return result;
}

# endif


/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_NCOPY

char *
str_ncopy(const char * src, uint32_t s)
{
    if (!src) return NULL;

    char * result = malloc((s+1)*sizeof(char));

    if (s)
        strncpy(result, src, s);

    result[s]=0;

    return result;
}

# endif
/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_PRINTF

#ifndef CSTUFF_STR_UTILS_WITH_VPRINTF
#define CSTUFF_STR_UTILS_WITH_VPRINTF
#endif

char *
str_printf(const char * format,  ...)
{
    va_list vl;
    va_start (vl, format);
    char * result = str_vprintf(format, vl);
    va_end(vl);

    return result;
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_VPRINTF

char *
str_vprintf(const char * format,  va_list vl)
{
    if (!format) return NULL;

    va_list _vl;
    va_copy(_vl, vl);

    uint32_t      total = 0,
                fmt_len = 0;

    char   tmp_buffer[64],
             tmp_format[64],
           * chr_arg;
    const char * pch = format;



    enum {
        wsFormatSearch,
        wsFormatFlags,
        wsFormatWidth,
        wsFormatPrecisionDot,
        wsFormatPrecision,
        wsFormatLength,
        wsFormatSpecifier
    };

    uint32_t stage = wsFormatSearch;

    while(*pch)
    {
        switch (stage)
        {
            case wsFormatSearch:
                if (*pch=='%')
                {
                    stage = wsFormatFlags;
                    strcpy(tmp_format, "%");
                    fmt_len=1;
                }
                else
                {
                    total++;
                }
                pch++;
                break;
            case wsFormatFlags:
                switch (*pch)
                {
                    case '-':
                    case '+':
                    case ' ':
                    case '#':
                    case '0':
                        tmp_format[fmt_len]=*pch;
                        fmt_len++;
                        pch++;
                    default:
                        stage=wsFormatWidth;
                }
                break;
            case wsFormatWidth:
                switch (*pch)
                {
                    case '*':
                        stage=wsFormatPrecisionDot;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        tmp_format[fmt_len]=*pch;
                        fmt_len++;
                        pch++;
                        break;
                    default:
                        stage=wsFormatPrecisionDot;
                }
                break;
            case wsFormatPrecisionDot:
                if (*pch=='.')
                {
                    tmp_format[fmt_len]=*pch;
                    fmt_len++;
                    pch++;
                    stage=wsFormatPrecision;
                }
                else
                    stage=wsFormatLength;
                break;
            case wsFormatPrecision:
                switch(*pch)
                {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        tmp_format[fmt_len]=*pch;
                        fmt_len++;
                        pch++;
                        break;
                    default:
                        stage=wsFormatLength;
                }
                break;
            case wsFormatLength:
                switch(*pch)
                {
                    case 'h':
                    case 'l':
                    case 'j':
                    case 'z':
                    case 't':
                    case 'L':
                        tmp_format[fmt_len]=*pch;
                        fmt_len++;
                        pch++;
                        break;
                    default:
                        stage=wsFormatSpecifier;

                }
                break;
            case wsFormatSpecifier:
                tmp_format[fmt_len]=*pch;
                fmt_len++;
                tmp_format[fmt_len]=0;
                stage = wsFormatSearch;

                switch (*pch)
                {
                    case '%':
                        total+=1;
                        break;
                    case 'd':
                    case 'i':
                    case 'u':
                    case 'o':
                    case 'x':
                    case 'X':
                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'a':
                    case 'A':
                    case 'c':
                    case 'p':
                    case 'n':
                        vsprintf(tmp_buffer, tmp_format, vl);
                        total+=strlen(tmp_buffer);
                        break;
                    case 's':
                        chr_arg = va_arg(vl, char*);
                        total+=chr_arg ? strlen( chr_arg ) : 6; /* (null) */
                        break;
                    default:
                        fprintf(
                            stderr, "Warning: "
                                    "bad format specifier (%s)\n", tmp_format
                        );
                        return NULL;
                }
                pch++;

                break;
        }

    }

    char * result = malloc(total+1);

    vsprintf(result, format, _vl);

    va_end(_vl);

    return result;
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_CAT

#ifndef CSTUFF_STR_UTILS_WITH_NCAT
#define CSTUFF_STR_UTILS_WITH_NCAT
#endif

char *
str_cat(char * source, const char * target)
{
    return str_ncat(source, target, strlen(target));
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_NCAT

char *
str_ncat(char * source, const char * target, uint32_t length)
{
  char * result = realloc( source,
                    length + ((source) ? strlen(source) : 0) + 1
                  );
  if (result)
  {
    if (!source)
      result[0]=0;

    if (target && length)
      strncat(result, target, length);

  }

  return result;
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_CMPI

#include <ctype.h>

int
str_cmpi(const char * source, const char * target)
{
  char r;
  while ( *source != 0 && *target != 0 )
  {
    r = tolower(*source) - tolower(*target);
    if ( r != 0 ) return r;
      source++;
      target++;
  }

  return ( *source == 0 && *target == 0 ) ? 0: -1;
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_TO_INT

int
str_to_int(const char * ptr, int l, int32_t * result)
{
  int64_t b;
  int rc = str_to_int64(ptr, l, &b);

  *result = b & 0xFFFFFFFF;

  return rc;
}

/* -------------------------------------------------------------------------- */

int
str_to_int64(const char * ptr, int l, int64_t * result)
{
  int    rc;
  char * src;
  int    dest;

  if ( (src = str_ncopy(ptr, l)) != NULL )
  {
    char * p;
    dest = strtoll(src, &p, 10);

    rc =  (p && *p == 0) ? CSTUFF_SUCCESS : CSTUFF_PARSE_ERROR;

    free(src);
  }
  else
  {
    dest = 0;
    rc = CSTUFF_MALLOC_ERROR;
  }

  *result = dest;

  return rc;
}

#endif


/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_TO_TIMESTAMP

#include <time.h>

int
str_to_timestamp(const char * source, size_t l, time_t * p_ts, const char * tz)
{
  int result;

  if (!tz)
    tz = "UTC";

  if ( ! (l < 19) )
  {
    struct tm * tms = calloc(1, sizeof(struct tm));
    if (tms)
    {
      char * curtz;

      tms->tm_year  = strtol(source,      NULL, 10) - 1900;
      tms->tm_mon   = strtol(&source[5],  NULL, 10) - 1;
      tms->tm_mday  = strtol(&source[8],  NULL, 10);
      tms->tm_hour  = strtol(&source[11], NULL, 10);
      tms->tm_min   = strtol(&source[14], NULL, 10);
      tms->tm_sec   = strtol(&source[17], NULL, 10);
      tms->tm_isdst = -1;

      curtz = getenv("TZ");
      setenv("TZ", tz, 1);

      *p_ts = mktime(tms);

      if (curtz)
        setenv("TZ", curtz, 1);
      else
        putenv("TZ");

      free(tms);

      result = 0;
    }
    else
      result = -1;
  }
  else
    result = 1;

  return result;
}

#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_FROM_TIMESTAMP

char *
str_from_timestamp_iso_utc(time_t ts)
{
  return str_from_timestamp(ts, CSTUFF_TIMESTAMP_FORMAT, "UTC");
}

#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_FROM_TIMESTAMP_FORMAT

char *
str_from_timestamp(time_t ts, const char * format, const char * timezone)
{
  char * result;
  char * cur_tz;

  if (!format)
    format = "%c";

  if (!timezone)
    timezone = "UTC";

  cur_tz = getenv("TZ");
  setenv("TZ", timezone, 1);

  struct tm * p_tm = localtime(&ts);

  if (cur_tz)
    setenv("TZ", cur_tz, 1);
  else
    putenv("TZ");

  if ((result = calloc(64, sizeof(char))) != NULL)
  {
    strftime(result, 64, format, p_tm);
  }

  return result;
}

#endif

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_CHOP

/* chop spaces from string, return number of prepending spaces */
uint32_t
str_chop(char * source)
{
  char * p = source;
  uint32_t   s = 0;

  /* remove ending spaces */
  s = strlen(source);
  while (--s)
  {
    if (source[s] != ' ') break;
      source[s] = 0;
  }

  s=0;
  /* remove prepending spaces */
  while (*p == ' ') s++;
  if (s)
    strcpy(source, &source[s]);

  return s;

}

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_REPLACE

char *
str_replace(const char * haystack, const char * needle, const char * value)
{
  size_t h_len,
         n_len = strlen(needle),
         v_len,
         match = 0;

  char * cur = (char *) haystack,
       * ptr,
       * result,
       * out;

  while ( (ptr = strstr(cur, needle)) != NULL )
  {
    cur = (char *)(ptr + n_len);
    match++;
  }

  if (!match)
    return (char*) haystack;

  v_len = strlen(value);
  h_len = strlen(haystack) - match * n_len + match * v_len + 1;

  if ((result = calloc(h_len, sizeof(char))) != NULL)
  {
    cur = (char *) haystack;
    out = result;

    while ( (ptr = strstr(cur, needle)) != NULL )
    {
      if (cur == haystack && ptr != haystack)
      {
        strncpy(result, haystack, (int)(ptr-haystack));
        out += (int)(ptr-haystack);
      }

      strncpy(out, value, v_len);

      out += v_len;

      cur = (char *)(ptr + n_len);
    }

    if (*cur)
      strcpy(out, cur);
  }

  return result;
}

# endif

/* -------------------------------------------------------------------------- */
