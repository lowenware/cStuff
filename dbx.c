/* dbx.c : source file of asynchronous PostgreSQL Database Engine
 * */
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>

#include "retcodes.h"
#include "list.h"
#include "str-utils.h"
#include "sock-utils.h"
#include "dbx.h"

/* -------------------------------------------------------------------------- */

static const char dbxUriFormat[] = "postgresql://%s:%s@%s:%d/%s",
                  dbxNullStr[]   = "NULL",
                  dbxTrueStr[]   = "TRUE",
                  dbxFalseStr[]  = "FALSE";

/* -------------------------------------------------------------------------- */

struct dbx_param
{
  const char   * value;
  int            length;
  dbx_param_t    type;
};

/* -------------------------------------------------------------------------- */

/* connection URI */
static char    * dbxUri;

/* -------------------------------------------------------------------------- */

/* connections pointers */
PGconn        ** dbxConnPool;

/* connections number */
static int       dbxConnSize;

/* connections number */
static int       dbxConnIter;

/* connection mask 1 = connected, 0 = not connected */
static uint64_t  dbxConnMask; 

/* business mask 1 = busy, 0 = available */
static uint64_t  dbxConnBusy; 

/* pgres polling and result counter */
static int     * dbxConnCell;

/* -------------------------------------------------------------------------- */

const char     * dbxErrorMessage;

/* -------------------------------------------------------------------------- */

/* requests queue */
static list_t    dbxQueue;

/* -------------------------------------------------------------------------- */

/* query identificator */
static uint64_t dbxQueryId;

/* dbx_request_t ------------------------------------------------------------ */

struct dbx_request 
{
  uint64_t           id;
  const char       * sql;
  char             * ptr;    /* will be free if is set */
  void             * u_data;
  PGconn           * conn;
  dbx_on_result_t    on_result;
  dbx_on_error_t     on_error;
};

typedef struct dbx_request * dbx_request_t;

static void
dbx_request_free(dbx_request_t req)
{
  if (req->ptr)
    free( (void *) req->ptr);
  free(req);
}

/* -------------------------------------------------------------------------- */

static dbx_request_t
dbx_queue_get( PGconn * conn )
{
  int             i = 0;
  dbx_request_t   req = NULL;

  while (i < dbxQueue->count)
  {
    req = list_index(dbxQueue, i);

    if ( req->conn == conn ) break;

    req = NULL;

    i++;
  }
  return req;
}


/* -------------------------------------------------------------------------- */

static void
dbx_queue_release( PGconn * conn )
{
  int             i = 0;
  dbx_request_t   req = NULL;

  while (i < dbxQueue->count)
  {
    req = list_index(dbxQueue, i);

    if ( req->conn == conn )
    {
      req->conn = NULL;
      break;
    }

    i++;
  }
}

/* -------------------------------------------------------------------------- */

static uint64_t
dbx_queue_add(const char * sql, dbx_on_result_t  on_result,
                                dbx_on_error_t   on_error, 
                                void            *u_data,
                                int              do_free )
{
  dbx_request_t r;

  if ( !(r = malloc(sizeof (struct dbx_request))) )
    return 0;

  r->id        = ++dbxQueryId ? dbxQueryId : ++dbxQueryId;
  r->sql       = sql;
  r->ptr       = (do_free ? (char*) sql : NULL);
  r->u_data    = u_data;
  r->on_result = on_result;
  r->on_error  = on_error;
  r->conn      = NULL;

  if (list_append(dbxQueue, r) == -1)
  {
    free(r);
    return 0;
  }

  return r->id;
}

/* -------------------------------------------------------------------------- */

cstuff_retcode_t
dbx_init( const char * username,
          const char * password,
          const char * database,
          const char * hostname,
          int          port,
          int          connections )
{
  dbxUri = str_printf( dbxUriFormat, 
                       username,
                       password,
                       hostname,
                       port,
                       database );

  if ( !dbxUri )
    goto e_malloc;

  if ( !(dbxQueue = list_new(4)) )
    goto release_uri;

  if (connections > (sizeof(dbxConnBusy)*8))
    dbxConnSize = sizeof(dbxConnBusy)*8;
  else if (connections < 1)
    dbxConnSize = 1;
  else
    dbxConnSize = connections;

  if ( !(dbxConnPool = calloc(dbxConnSize, sizeof(void*))) )
    goto release_queue;

  if ( !(dbxConnCell = malloc(dbxConnSize*sizeof(PostgresPollingStatusType))) )
    goto release_list;

  dbxConnBusy     = 0;
  dbxConnMask     = 0;
  dbxConnIter     = 0;
  dbxQueryId      = 0;
  dbxErrorMessage = NULL;

  return CSTUFF_SUCCESS;

release_list:
  free(dbxConnPool);

release_queue:
  list_free(dbxQueue, free);

release_uri:
  free(dbxUri);
  dbxUri = NULL;

e_malloc:
  return CSTUFF_MALLOC_ERROR;
}

/* -------------------------------------------------------------------------- */

void
dbx_release()
{
  int i;

  if (dbxUri)
  {
    free(dbxUri);
    dbxUri = NULL;
  }

  if (dbxQueue)
  {
    list_free(dbxQueue, (list_destructor_t) dbx_request_free);
    dbxQueue = NULL;
  }

  if (dbxConnPool)
  {
    for (i=0; i<dbxConnSize; i++)
    {
      if (dbxConnPool[i])
        PQfinish( dbxConnPool[i] );

    }

    free( dbxConnPool );
    dbxConnPool = NULL;
  }

  if (dbxConnCell)
  {
    free( dbxConnCell );
    dbxConnCell = NULL;
  }
}

/* -------------------------------------------------------------------------- */

static cstuff_retcode_t
dbx_touch_connection( PGconn * conn )
{
  PGresult     * res;
  dbx_request_t  req;

  if ( !(dbxConnBusy & (1<<dbxConnIter)) )
  {
    if ( dbxQueue->count )
    {
      if ( (req = dbx_queue_get( NULL )) != NULL )
      {
        if ( PQsendQuery(conn, req->sql) )
        {
          /* set request handling */
          req->conn = conn;
          /* set connection busy */
          dbxConnBusy |= (1<<dbxConnIter);
          /* reset result counter */
          dbxConnCell[ dbxConnIter ] = 0;

        }
        else
          return CSTUFF_EXTCALL_ERROR;
      }
    }
    else
    {
      if ( !dbxConnBusy )
        return CSTUFF_PENDING;
    }

    return CSTUFF_SUCCESS;
  }

  /* there is already some request handled via connection */

  req = dbx_queue_get( conn );

  if (sock_has_input( PQsocket(conn), 50))
  {
    if (PQconsumeInput(conn))
    {
      if (PQisBusy(conn))
        return CSTUFF_SUCCESS;

      while ((res = PQgetResult(conn)) != NULL)
      {
        switch (PQresultStatus(res))
        {
          case PGRES_COMMAND_OK:
          case PGRES_TUPLES_OK:
            if (req->on_result)
            {
              if (!(req->on_result(res, dbxConnCell[dbxConnIter], req->u_data)))
                break;
            }
            PQclear(res);
            break;

          default:
            if (req->on_error)
            {
              req->on_error(
                PQresultErrorMessage(res),
                dbxConnCell[dbxConnIter],
                req->u_data,
                req->sql
              );
            }

            PQclear(res);
        }
        dbxConnCell[dbxConnIter]++;
      }

      list_remove(dbxQueue, req);
      dbx_request_free(req);

      dbxConnBusy ^= (1<<dbxConnIter);
    }
    else
      return CSTUFF_EXTCALL_ERROR;
  }

  return CSTUFF_SUCCESS;
}

/* -------------------------------------------------------------------------- */

cstuff_retcode_t
dbx_touch()
{
  cstuff_retcode_t result = CSTUFF_PENDING;

  if ( ! dbxConnPool[ dbxConnIter ] ) /* not assigned yet */
  {
    if ( !(dbxConnPool[ dbxConnIter ] = PQconnectStart(dbxUri)) )
    {
      result = CSTUFF_MALLOC_ERROR;
      goto finally;
    }
    dbxConnCell[ dbxConnIter ] = PGRES_POLLING_WRITING;
  }

  switch(PQstatus(dbxConnPool[dbxConnIter]))
  {
    case CONNECTION_OK:
      dbxConnMask |= (1<<dbxConnIter);
/*      printf( "(dbxConnMask |= (1<<dbxConnIter)) == %"PRIu64"\n", dbxConnMask); */
      result = dbx_touch_connection( dbxConnPool[dbxConnIter] );
      break;

    case CONNECTION_BAD:
      if (dbxConnMask & (1<<dbxConnIter))
        dbxConnMask ^= (1<<dbxConnIter);

      if ( dbxConnBusy & (1<<dbxConnIter) ) /* release request */
      {
        /* make sure we've released all querries */
        dbx_queue_release( dbxConnPool[ dbxConnIter ] );
        /* unset busy flag to use proper poll function */
        dbxConnBusy ^= (1<<dbxConnIter);
      }

      PQfinish(dbxConnPool[dbxConnIter]);
      dbxConnPool[dbxConnIter] = NULL;

      dbxErrorMessage = PQerrorMessage( dbxConnPool[dbxConnIter] );

      result = CSTUFF_EXTCALL_ERROR;
      break;

    default:
      result = CSTUFF_SUCCESS;

      switch( dbxConnCell[ dbxConnIter ] )
      {
        case PGRES_POLLING_FAILED:
          result = CSTUFF_EXTCALL_ERROR;
          goto finally;

        case PGRES_POLLING_READING:
          if (sock_has_input( PQsocket(dbxConnPool[dbxConnIter]), 50) < 1)
            goto finally;
          break;

        case PGRES_POLLING_WRITING:
          if (sock_can_write( PQsocket(dbxConnPool[dbxConnIter]), 50) < 1)
            goto finally;
          break;

        case PGRES_POLLING_OK:
        case PGRES_POLLING_ACTIVE:
          goto finally;
      }

      dbxConnCell[ dbxConnIter ] = PQconnectPoll(dbxConnPool[dbxConnIter]);
  }


finally: /* always iterate connection */
  if ( !(++dbxConnIter < dbxConnSize) )
    dbxConnIter = 0;

  return result;
}

/* -------------------------------------------------------------------------- */

static void
dbx_query_int32_to_chars( int32_t            value,
                          char            ** p_chars,
                          const char       * format )
{
  if ( (*p_chars = malloc(11)) != NULL)
    sprintf( *p_chars, format, value);
}

/* -------------------------------------------------------------------------- */

static void
dbx_query_int64_to_chars( int64_t            value,
                          char            ** p_chars,
                          const char       * format )
{
  if ( (*p_chars = malloc(21)) != NULL)
    sprintf( *p_chars, format, value);
}

/* -------------------------------------------------------------------------- */

static PGconn *
dbx_get_allocated_connection()
{
  int i=0;

  while (i<dbxConnSize)
  {
    if ( dbxConnPool[i] )
      return dbxConnPool[i];
    i++;
  }

  return NULL;
}

/* -------------------------------------------------------------------------- */

static cstuff_retcode_t
dbx_query_args_to_list( va_list a_list, int p_count, struct dbx_param * p_list )
{
  int      i, j;
  char   * ch_ptr,
         * value;
  PGconn * conn = NULL;


  for (i=0; i<p_count; i++)
  {
    value          = NULL;
    p_list[i].type = va_arg(a_list, int);

    switch( p_list[i].type )
    {
      case DBX_INT32:
        dbx_query_int32_to_chars(va_arg(a_list,int32_t), &value, "%d");
        break;

      case DBX_UINT32:
        dbx_query_int32_to_chars(va_arg(a_list,int32_t), &value, "%d");
        break;

      case DBX_INT64:
        dbx_query_int64_to_chars(va_arg(a_list,int64_t), &value, "%"PRId64);
        break;

      case DBX_UINT64:
        dbx_query_int64_to_chars(va_arg(a_list,int64_t), &value, "%"PRIu64);
        break;

      case DBX_CONSTANT:
        if ((ch_ptr = va_arg(a_list, char *)) != NULL)
        {
          if ( (value = malloc( strlen(ch_ptr)+2+1 )) != NULL)
          {
            sprintf(value, "'%s'", ch_ptr);
          }
        }
        else
          value = (char *) dbxNullStr;

        break;

      case DBX_STATEMENT:
        ch_ptr = va_arg(a_list, char *); 
        value =  ch_ptr ? ch_ptr : (char*) dbxNullStr;
        break;

      case DBX_STRING:
        if ((ch_ptr = va_arg(a_list, char *)) != NULL)
        {
          if (!conn) 
            conn = dbx_get_allocated_connection();

          value = PQescapeLiteral(conn, ch_ptr, strlen(ch_ptr));
        }
        else
          value = (char *) dbxNullStr;

        break;

      case DBX_MD5_HASH:
        ch_ptr = va_arg(a_list, char *);
        if ( (value = malloc( (32+2+1) * sizeof(char))) != NULL )
        {
          value[0] ='\'';

          for (j = 0; j < 16; j++)
          {
            sprintf(&value[1+j*2], "%02x", ch_ptr[j] & 0xFF);
          }

          value[33]='\'';
          value[34]='0';
        }
        break;

      case DBX_BOOLEAN:
        value = (char*) (va_arg(a_list, int) ? dbxTrueStr : dbxFalseStr);
        break;
    }

    if ( value )
    {
      p_list[i].length = strlen( value );
      p_list[i].value  = value;
    }
    else
       return CSTUFF_MALLOC_ERROR;

  }

  return CSTUFF_SUCCESS;
}

/* -------------------------------------------------------------------------- */

char *
dbx_sql_format( const char      * sql_format,
                int               p_count,
                                  ... )
{
  char * result;
  va_list args;

  va_start(args, p_count);
  result = dbx_sql_vformat(sql_format, p_count, args);
  va_end(args);

  return result;
}

/* -------------------------------------------------------------------------- */

char *
dbx_sql_vformat( const char      * sql_format,
                 int               p_count,
                 va_list           a_list )
{
  struct dbx_param  * p_list  = NULL;              /* parameters array */
  char              * ptr     =(char*) sql_format, /* pointer iterator */
                    * sql = NULL;                  /* result sql */
  int                 i,                           /* index / iterator */
                      l       = 0;                 /* length / index */

  /* prepare list for params */
  if ( !(p_list = calloc(p_count, sizeof(struct dbx_param))) )
    goto finally;

  /* preapare params */
  if (dbx_query_args_to_list( a_list, p_count, p_list ) != CSTUFF_SUCCESS)
    goto release;

  /* calculate final size */
  while (*ptr)
  {
    if ( (*ptr == '$') && (i = strtol(ptr+1, NULL, 10)) > 0)
      l += p_list[ i-1 ].length;

    ptr++;
  }
  
  l += ((int) (ptr - sql_format)+1);

  /* make sql string */
  if ( !(sql = malloc( l*sizeof(char))) )
    goto release;

  ptr = (char*) sql_format;
  l   = 0;

  while ( *ptr )
  {
    if ( (*ptr == '$') && (i = strtol( ptr+1, &ptr, 10)) > 0 )
    {
      i--;
      strncpy( &sql[l], p_list[i].value, p_list[ i ].length);
      l += p_list[ i ].length;
      continue; /* new value of ptr already set */
    }

    sql[l++] = *(ptr++);
  }

  /* terminate string with null */
  sql[l] = 0;


release:
  for ( i=0; i<p_count; i++)
  {
    if ( ! p_list[i].value ) continue;

    switch( p_list[i].type )
    {
      case DBX_STRING:
        if ( p_list[i].value != dbxNullStr )
          PQfreemem( (char *) p_list[i].value );
        break;

      case DBX_BOOLEAN:
      case DBX_STATEMENT:
        break;

      case DBX_CONSTANT:
        if ( p_list[i].value == dbxNullStr )
          break;

      default:
        free( (char*) p_list[i].value );
    }

  }
  free(p_list);

finally:
  return sql;

}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

uint64_t
dbx_query_format( const char      * sql_format,
                  dbx_on_result_t   on_result,
                  dbx_on_error_t    on_error, 
                  void            * u_data,
                  int               p_count,
                                    ... )
{
  uint64_t   result;         /* result: 0 -fail */
  char     * sql;            /* result sql */
  va_list    args;

  va_start(args, p_count);
  sql = dbx_sql_vformat(sql_format, p_count, args);
  va_end(args);

  if (sql)
  {
    result = dbx_queue_add(sql, on_result, on_error, u_data, 1);
    if (!result)
      free(sql);
  }
  else
    result = 0;

  return result;
}

/* -------------------------------------------------------------------------- */

uint64_t
dbx_query_const( const char      * sql,
                 dbx_on_result_t   on_result,
                 dbx_on_error_t    on_error, 
                 void            * u_data )
{
  uint64_t   result;         /* result: 0 -fail */
  result = dbx_queue_add(sql, on_result, on_error, u_data, 0);

  return result;
}

/* -------------------------------------------------------------------------- */

uint64_t
dbx_query_transaction( const char      * sql,
                       dbx_on_result_t   on_result,
                       dbx_on_error_t    on_error, 
                       void            * u_data )
{
  uint64_t   result;         /* result: 0 -fail */
  char     * t_sql;

  if ( (t_sql = str_printf("BEGIN;\n%sCOMMIT;\n", sql)) != NULL )
  {
    result = dbx_queue_add(t_sql, on_result, on_error, u_data, 1);
    if (!result)
      free(t_sql);
  }
  else
    result = 0;

  return result;
}


/* -------------------------------------------------------------------------- */

cstuff_retcode_t
dbx_cancel( uint64_t id )
{
  int i;
  dbx_request_t req;
  for (i=0; i<dbxQueue->count; i++)
  {
    req = (dbx_request_t) list_index(dbxQueue, i);
    if ( req->id == id)
    {
      if ( req->conn )
      {
        /* already pending */
        req->on_result = NULL;
        req->on_error  = NULL;
      }
      else
      {
        /* just in queue */
        dbx_request_free( req );
        list_remove_index( dbxQueue, i );
      }
      return CSTUFF_SUCCESS;
    }
  }

/* -------------------------------------------------------------------------- */
  return CSTUFF_NOT_FOUND;
}

/* -------------------------------------------------------------------------- */

const char *
dbx_get_error()
{
  return dbxErrorMessage;
}

/* -------------------------------------------------------------------------- */

int
dbx_ready_connections_count()
{
  uint64_t mask   = dbxConnMask;
  int      result = 0;

  while (mask)
  {
    if ( mask & 1 )
      result++;
    mask = mask >> 1;
  }

  return result;
}

/* -------------------------------------------------------------------------- */

int
dbx_as_timestamp( PGresult * data, int row_num, int col_num, time_t * p_ts )
{
  int result = -1;

  struct tm * tms = calloc(1, sizeof(struct tm));

  if (tms)
  {
    char * dt = dbx_as_string(data, row_num, col_num);
    if (dt)
    {
      int l = strlen(dt);

      if ( ! (l < 19) )
      {
        char * curtz;

        tms->tm_year  = strtol(dt,      NULL, 10) - 1900;
        tms->tm_mon   = strtol(&dt[5],  NULL, 10) - 1;
        tms->tm_mday  = strtol(&dt[8],  NULL, 10);
        tms->tm_hour  = strtol(&dt[11], NULL, 10);
        tms->tm_min   = strtol(&dt[14], NULL, 10);
        tms->tm_sec   = strtol(&dt[17], NULL, 10);
        tms->tm_isdst = -1;

        curtz = getenv("TZ");
        setenv("TZ", "UTC", 1);

        *p_ts = mktime(tms);

        if (curtz)
          setenv("TZ", curtz, 1);
        else
          putenv("TZ");

        result = 0;
      }
    }

    free(tms);
  }

  return result;
}
