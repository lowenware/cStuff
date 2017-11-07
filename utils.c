#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <common/utils.h>

/* String Commons ----------------------------------------------------------- */

char *
u_strdup(const char * src)
{
    if (!src) return NULL;

    uint32_t s = strlen(src);
    char * result = malloc((s+1)*sizeof(char));

    strcpy(result, src);

    return result;
}

char *
u_strset(char * init, const char * src)
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
                u_free(init);
            }

        }
        else
        {
            u_free(init);
            return NULL;
        }
    }

    return u_strdup(src);
}

char *
u_strndup(const char * src, uint32_t s)
{
    if (!src) return NULL;

    char * result = malloc((s+1)*sizeof(char));

    if (s)
        strncpy(result, src, s);
    
    result[s]=0;

    return result;
}

/* -------------------------------------------------------------------------- */

char *
u_vsprintf(const char * format,  va_list vl)
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
                            stderr, "WebStuff Warning: "
                                    "bad format specifier (%s)\n", tmp_format
                        );
                        return NULL;
                }
                pch++;

                break;
        }

    }

    char * result = u_malloc(total+1);

    vsprintf(result, format, _vl);

    va_end(_vl);

    return result;
}

/* -------------------------------------------------------------------------- */

char *
u_sprintf(const char * format,  ...)
{
    va_list vl;
    va_start (vl, format);
    char * result = u_vsprintf(format, vl);
    va_end(vl);

    return result;
}

/* -------------------------------------------------------------------------- */


char *
u_strcat(char * source, const char * target)
{
    return u_strncat(source, target, strlen(target));
}

/* -------------------------------------------------------------------------- */

char *
u_strncat(char * source, const char * target, uint32_t length)
{
    char * result = u_malloc(
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

/* networking --------------------------------------------------------------- */

void
u_inetaddr_to_str(struct sockaddr_in * sa, char * sa_str)
{
  inet_ntop( AF_INET, &sa->sin_addr, sa_str, INET_ADDRSTRLEN );
  sprintf(
    &sa_str[strlen(sa_str)], ":%d", (int) ( ntohs(sa->sin_port) ) & 0xFFFF
  );
}

/* Timer -------------------------------------------------------------------- */
/*

void
u_stopwatch_start(WsStopwatch * sw)
{
    time(&sw->begin);
    sw->end   = 0;
}


void
u_stopwatch_stop(WsStopwatch * sw)
{
    time(&sw->end);
}


uint32_t
u_stopwatch_get_elapsed(WsStopwatch * sw)
{
    return difftime( (sw->end==0) ? time(NULL) : sw->end, sw->begin );
}

*/


void
u_sleep(uint32_t usec)
{
    #ifdef WIN32
    sleep(usec);
    #else
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = usec*1000;
    select(0, NULL, NULL, NULL, &tv);
    #endif
}

/* Timestamp */

/*
static int
get_digit(char * p, int size)
{
    char buf[size+1];
    buf[size]=0;
    strncpy(buf, p, size);

    return atoi(buf);
}


WsTimestamp *
u_now(WsTimestamp * ts, WsTimezone tz)
{
    time_t rawtime;
    time ( &rawtime );

    if (!ts)
        ts = u_malloc(sizeof(WsTimestamp));


    if (tz == wsUTC)
        gmtime_r(&rawtime, ts);
    else
        localtime_r ( &rawtime, ts );


    return ts;
}

WsTimestamp *
u_timestamp_from_ascii(WsTimestamp * ts, char * string)
{
    / * Ascii time: Tue Sep 15 16:27:37 2015 +0200 * /
    if (!string) return NULL;
    / * get length * /
    uint32_t s = strlen(string);
    / * bad ascii time * /
    if (s<24) return NULL;
    / * prepare time structure * /
    if (!ts) ts=calloc(1, sizeof(WsTimestamp));

    char * p;

    const char months[][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    p = &string[ 19 ];
    if (*p==' ') p++;

    ts->tm_year = get_digit(p, 4) - 1900;


    for (ts->tm_mon=0; ts->tm_mon<11; ts->tm_mon++)
    {
        if (strncmp(months[ts->tm_mon], &string[4], 3)==0) break;
    }
    ts->tm_mday = get_digit(&string[8], 2);
    ts->tm_hour = get_digit(&string[11], 2);
    ts->tm_min  = get_digit(&string[14], 2);
    ts->tm_sec  = get_digit(&string[17], 2);

    mktime ( ts );

    return ts;
}


void
u_timestamp_set_from_string(WsTimestamp * ts, char * string)
{
    if (!string || !ts) return;
    int l = strlen(string);

    char * e;

    if (l > 9) / * YYYY-MM-DD * /
    {
        e = strchr(string, '-');

        / * get current timeinfo and modify it according to string * /
        ts->tm_year = get_digit(string, (int)(e-string)) - 1900;
        ts->tm_mon  = get_digit(e+1, 2)- 1;
        ts->tm_mday = get_digit(e+4, 2);
        if (l > 18) / * YYYY-MM-DD HH:MM:SS * /
        {
            ts->tm_hour = get_digit(e+7, 2);
            ts->tm_min  = get_digit(e+10, 2);
            ts->tm_sec  = get_digit(e+13, 2);
        }
        else
        {
            ts->tm_hour = 0;
            ts->tm_min  = 0;
            ts->tm_sec  = 0;
        }
    }

    mktime ( ts );
}


void
u_timestamp_set_string(WsTimestamp * ts, WsString * string)
{
    / * Length(YYYY-MM-DD HH:MM:SS)=19, for strftime we have to pass 20 because
     * of NULL-termination, that is added automatically 
     * by u_string_set_length
     * * /
    u_string_set_length(string, 19); 
    strftime (*string, 20, "%F %T", ts);
}

void
u_timestamp_add_seconds(WsTimestamp * ts, WsInt seconds)
{
    ts->tm_sec += seconds;
    mktime(ts);
}

*/


/* Free --------------------------------------------------------------------- */

void *
u_malloc(uint32_t size)
{
    void * result = malloc(size);
    if (!result)
    {
        fprintf(stderr, "WebStuff: Fatal Error (out of memory)");
        exit(EXIT_FAILURE);
    }
    return result;
}

void *
u_calloc(uint32_t length, uint32_t size)
{
    void * result = calloc(length, size);
    if (!result)
    {
        fprintf(stderr, "WebStuff: Fatal Error (out of memory)");
        exit(EXIT_FAILURE);
    }
    return result;
}

void
u_free(void * ptr)
{
    if (ptr)
        free(ptr);
}

void *
u_nfree(void * ptr)
{
    if(ptr)
        free(ptr);
    return NULL;
}
