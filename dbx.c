#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>

#include "list.h"
#include "sock-utils.h"
#include "str-utils.h"
#include "dbx.h"

/* type --------------------------------------------------------------------- */

typedef PostgresPollingStatusType (*pg_poll_t)(PGconn *conn);

struct _param
{
  char * str;
  int    len;
};


/* globals ------------------------------------------------------------------ */

/* postgresql connection */
PGconn *dbxConn  = NULL;

/* requests queue */
static list_t dbx_queue = NULL;

/* postgresql connection socket */
static int dbx_sock;

/* function to poll connection PQconnectPoll or PQresetPoll */
static pg_poll_t dbx_poll_func;

/* result of last poll function call */
static PostgresPollingStatusType dbx_poll_type;

/* request is pending identifier */
static bool dbx_is_pending;

static uint64_t dbxQueryId = 0;


static void
dbx_reset_globals(pg_poll_t poll_func)
{
  dbx_poll_func  = poll_func;
  dbx_poll_type  = PGRES_POLLING_WRITING;
  dbx_is_pending = false;
  dbx_sock       = PQsocket(dbxConn);
}


/* connection uri to be set up elswhere */
char  *dbxUri = NULL;


/* dbx_request_t ------------------------------------------------------------ */

struct _dbx_request_t 
{
  uint64_t           id;
  char              *sql;
  void              *u_data;
  dbx_on_result_t    on_result;
  dbx_on_error_t     on_error;
};

typedef struct _dbx_request_t * dbx_request_t;

static void
dbx_request_free(dbx_request_t req)
{
  free( (void *) req->sql);
  free(req);
}



/* init subsystem ----------------------------------------------------------- */

bool
dbx_init()
{
  
  if (dbx_queue) return true;

  dbx_queue = list_new(1);

  return (dbx_queue) ? true : false;
}

/* release used resources --------------------------------------------------- */

void
dbx_release()
{
  if (dbx_queue)
  {
    list_free(dbx_queue, (list_destructor_t) dbx_request_free);
    dbx_queue = NULL;
  }
}

/* refresh subsystem data --------------------------------------------------- */

dbx_status_t
dbx_touch(bool reconnect, bool is_error)
{
  PGresult *pg_result;
  static dbx_request_t r;
  static int results_count;

  if (reconnect)
  {
    if (dbxConn)
      PQfinish(dbxConn);

    dbxConn = PQconnectStart(dbxUri);
    dbx_reset_globals( PQconnectPoll );
  }

  if (!dbxConn) return DBX_MALLOC_ERROR;

  switch(PQstatus(dbxConn))
  {
    case CONNECTION_OK:
      /* handle requests */
      if (dbx_is_pending)
      {
        if (sock_has_input(dbx_sock, 100))
        {
          if (PQconsumeInput(dbxConn))
          {
            if (PQisBusy(dbxConn))
              return DBX_SUCCESS;

            while ((pg_result = PQgetResult(dbxConn)) != NULL)
            {
              switch (PQresultStatus(pg_result))
              {
                case PGRES_COMMAND_OK:
                case PGRES_TUPLES_OK:
                  if (!r->on_result || 
                       r->on_result(pg_result, results_count, r->u_data))
                    PQclear(pg_result);
                  break;
                default:

                  if (r->on_error)
                    r->on_error(
                      PQresultErrorMessage(pg_result),
                      results_count, 
                      r->u_data
                    );

                  PQclear(pg_result);
              }
              results_count++;
            }

            dbx_request_free(r);
            list_remove_index(dbx_queue, 0);
            dbx_is_pending = false;
          }
          else
          {
            fprintf(stderr, "[DBX] could not consume input from DB\n");
            return DBX_CONNECTION_ERROR;
          }
        }
      }
      else if (dbx_queue->count)
      {
        printf("get request from queue\n");
        r = list_index(dbx_queue, 0);
        if (!PQsendQuery(dbxConn, r->sql))
        {
          fprintf(stderr, "[DBX] could not send query to DB\n");
          return DBX_CONNECTION_ERROR;
        }
        dbx_is_pending = true;
        results_count = 0;
      }

      return DBX_SUCCESS;

    case CONNECTION_BAD:
      if (is_error)
      {
        PQresetStart(dbxConn);
        dbx_reset_globals( PQconnectPoll );
        return DBX_CONNECTING;
      }
      else
        return DBX_CONNECTION_ERROR;

    default:

      switch (dbx_poll_type)
      {
        case PGRES_POLLING_OK:
          return DBX_SUCCESS;

        case PGRES_POLLING_FAILED:
          return DBX_CONNECTION_ERROR;
        
        case PGRES_POLLING_READING:
          if (sock_has_input(dbx_sock,100) < 1)
            return DBX_CONNECTING;
          break;
        case PGRES_POLLING_WRITING:
          if (sock_can_write(dbx_sock,100) < 1)
            return DBX_CONNECTING;
          break;
        case PGRES_POLLING_ACTIVE:
          return DBX_CONNECTING;
      }
      dbx_poll_type = dbx_poll_func(dbxConn);
  }
  return DBX_CONNECTING;
}


/* query request ------------------------------------------------------------ */

uint64_t
dbx_query(char * sql, dbx_on_result_t  on_result,
                      dbx_on_error_t   on_error, 
                      void            *u_data)
{
  if (! sql || ! on_result) return 0;

  dbx_request_t r = malloc(sizeof (struct _dbx_request_t));

  if (!r) return 0;

  r->id        = ++dbxQueryId ? dbxQueryId : ++dbxQueryId;
  r->sql       = sql;
  r->u_data    = u_data;
  r->on_result = on_result;
  r->on_error  = on_error;

  list_append(dbx_queue, r);

  
  return r->id;
}

uint64_t
dbx_format_query( const char      *sql_format,
                  dbx_on_result_t  on_result,
                  dbx_on_error_t   on_error, 
                  void            *u_data,
                  int              p_count,
                  ... )
{
  /* prepare params array */
  struct _param * p = calloc(p_count, sizeof(struct _param));
  if (!p) return 0;

  int64_t     v_int;
  char      * v_chars,
            * sql,
              buffer[32+1];
  int         i, l, t;

  /* preapare params */
  va_list arg;
  va_start(arg, p_count);

  for (i=0; i<p_count; i++)
  {
    switch( va_arg(arg, int) )
    {
      case DBX_INT:
        v_int = va_arg(arg, int);
        sprintf(buffer, "%d", (int) (v_int & 0xFFFFFFFF));
        p[i].str = str_copy(buffer);
        break;

      case DBX_UINT:
        v_int = va_arg(arg, int);
        sprintf(buffer, "%u", (unsigned int) (v_int & 0xFFFFFFFF));
        p[i].str = str_copy(buffer);
        break;

      case DBX_INT64:
        v_int = va_arg(arg, int64_t);
        sprintf(buffer, "%"PRId64, v_int);
        p[i].str = str_copy(buffer);
        break;

      case DBX_UINT64:
        v_int = va_arg(arg, uint64_t);
        sprintf(buffer, "%"PRIu64, (uint64_t) v_int);
        p[i].str = str_copy(buffer);
        break;

      case DBX_CONSTANT:
        v_chars = va_arg(arg, char *);
        v_int = strlen(v_chars)+2+1;
        p[i].str = malloc(v_int);
        if (p[i].str)
          sprintf(p[i].str, "'%s'", v_chars);
        break;

      case DBX_STRING:
        v_chars = va_arg(arg, char *);
        
        p[i].str = (v_chars) ?
                      PQescapeLiteral( dbxConn, v_chars, strlen(v_chars) ) :
                      str_copy("NULL");
        break;

      case DBX_MD5_HASH:
        v_chars = va_arg(arg, char *);
        p[i].str = malloc( (32+2+1) * sizeof(char));
        if (p[i].str)
        {
          for (v_int = 0; v_int < 16; v_int++)
          {
            sprintf(&buffer[v_int*2], "%02x", v_chars[v_int] & 0xFF);
          }
          buffer[32] = 0;
          sprintf(p[i].str, "'%s'", buffer);
        }
        break;

      case DBX_BOOLEAN:
        v_int = va_arg(arg, int);
        sprintf(buffer, "%s", v_int ? "TRUE" : "FALSE");
        p[i].str = str_copy(buffer);
        break;
    }
    p[i].len = (p[i].str) ? strlen( p[i].str ) : 0;
  }
  va_end(arg);

  /* calculate final size */
  l = strlen( sql_format );
  t = l;
  for ( i=0; i < l; i++ )
  {
    if (sql_format[i] == '$')
    {
      v_int = strtol(&sql_format[i+1], NULL, 10);
      if (v_int-- && v_int < p_count)
      {
        t += p[ v_int ].len;
      }
    }
  }


  /* make string */
  sql = calloc((size_t)t &0xFFFF, sizeof(char));
  char * sql_p = sql;

  for ( i=0; i < l; i++ )
  {
    if (sql_format[i] == '$')
    {
      v_int = strtol(&sql_format[i+1], &v_chars, 10);
      if (v_int-- && v_int < p_count)
      {
        strncpy(sql_p, p[v_int].str, p[ v_int ].len);
        sql_p += p[ v_int ].len;
        i += (v_chars - &sql_format[i]);
      }
    }

    *(sql_p++) = sql_format[i];
  }

  /* free resources */

  for ( i=0; i< p_count; i++)
  {
    if (p[i].str)
      free(p[i].str);
  }
  free(p);

  /* printf ("FORMATED SQL: %s :: %d\n", sql, t); */

  return dbx_query(sql, on_result, on_error, u_data);

}



uint64_t
dbx_cancel(uint64_t id)
{
  if (!dbx_queue) return 0;

  int i;
  for (i=0; i<dbx_queue->count; i++)
  {
    if (((dbx_request_t) list_index(dbx_queue, i))->id == id)
    {
      ((dbx_request_t) list_index(dbx_queue, i))->on_result = NULL;
      ((dbx_request_t) list_index(dbx_queue, i))->on_error  = NULL;
      return id;
    }
  }
  return 0;
}

/* get error message -------------------------------------------------------- */


char *
dbx_get_error()
{
  return (dbxConn) ? PQerrorMessage(dbxConn) : "not allocated";
}

/* -------------------------------------------------------------------------- */

