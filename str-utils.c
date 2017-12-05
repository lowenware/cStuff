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
  
#include "str_utils.h"

/* -------------------------------------------------------------------------- */

#ifdef STR_UTILS_WITH_SET

#ifndef STR_UTILS_WITH_COPY
#define STR_UTILS_WITH_COPY
#endif

char *
str_set(char * init, const char * src)
{
    if (init)
    {
        if (src)
        {
            if (strlen(init) >= strlen(src))
            {
                strcpy(init, src);
                return init;
            }
            else
            {
                free(init);
            }

        }
        else
        {
            free(init);
            return NULL;
        }
    }

    return str_copy(src);
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef STR_UTILS_WITH_COPY

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

#ifdef STR_UTILS_WITH_NCOPY

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

#ifdef STR_UTILS_WITH_PRINTF

#ifndef STR_UTILS_WITH_VPRINTF
#define STR_UTILS_WITH_VPRINTF
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

#ifdef STR_UTILS_WITH_VPRINTF

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

#ifdef STR_UTILS_WITH_CAT

#ifndef STR_UTILS_WITH_NCAT
#define STR_UTILS_WITH_NCAT
#endif

char *
str_cat(char * source, const char * target)
{
    return str_ncat(source, target, strlen(target));
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef STR_UTILS_WITH_NCAT

char *
str_ncat(char * source, const char * target, uint32_t length)
{
    char * result = malloc(
                         length + ((source) ? strlen(source) : 0) + 1
                      );

    if (source)
    {
        strcpy(result, source);
        free(source);
    }
    else
        result[0]=0;

    if (target && length)
        strncat(result, target, length);


    return result;
}

# endif

/* -------------------------------------------------------------------------- */

#ifdef STR_UTILS_WITH_CMPI

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

#ifdef STR_UTILS_WITH_TO_INTEGER

uint64_t
str_to_integer(const char * ptr, int l)
{
  char buffer[20+1];
  if (l>20) l = 20; /* 20 is length of 2^64 */

  memcpy(buffer, ptr, l);
  buffer[l]=0;

  return strtoll(buffer, NULL, 10);
}

#endif

/* -------------------------------------------------------------------------- */
#ifdef STR_UTILS_WITH_CHOP

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
