#ifndef _STANDARD_DBX_H_
#define _STANDARD_DBX_H_

#include <stdbool.h>
#include <string.h>
#include <libpq-fe.h>

/* type --------------------------------------------------------------------- */

typedef enum
{
  DBX_SUCCESS,

  DBX_CONNECTING,
  DBX_CONNECTION_ERROR,
  
  DBX_REQUEST_ERROR,
  DBX_MALLOC_ERROR

} dbx_status_t;

typedef enum
{
  DBX_INT,
  DBX_UINT,
  DBX_INT64,
  DBX_UINT64,

  DBX_STRING,
  DBX_CONSTANT,
  DBX_MD5_HASH,

  DBX_BOOLEAN

} dbx_param_t;

typedef bool /* true - free result, false - keep result */
(*dbx_on_result_t)(PGresult *result, int res_i, void *u_data);

typedef void
(*dbx_on_error_t)(const char *error_message, int res_i, void *u_data);


extern char   *dbxUri;
extern PGconn *dbxConn;

#define dbx_escape(x) PQescapeLiteral(dbxConn, x, strlen(x))
#define dbx_free(x)   PQfreemem(x)

/* init subsystem ----------------------------------------------------------- */

bool
dbx_init();

/* release used resources --------------------------------------------------- */

void
dbx_release();

/* refresh subsystem data --------------------------------------------------- */

dbx_status_t
dbx_touch(bool reconnect, bool is_error);

/* query request ------------------------------------------------------------ */

uint64_t
dbx_query( char            *sql,
           dbx_on_result_t  on_result,
           dbx_on_error_t   on_error, 
           void            *u_data );

uint64_t
dbx_format_query( const char      *sql_format,
                  dbx_on_result_t  on_result,
                  dbx_on_error_t   on_error, 
                  void            *u_data,
                  int              p_count,
                  ... );

uint64_t
dbx_cancel(uint64_t id);

/* get error message -------------------------------------------------------- */

char *
dbx_get_error();

/* -------------------------------------------------------------------------- */

#define dbx_as_string( pg_result, row_num, col_num ) \
        PQgetvalue(pg_result, row_num, col_num)

#define dbx_as_integer( pg_result, row_num, col_num ) \
        strtol(PQgetvalue(pg_result, row_num, col_num), NULL, 10)

#define dbx_as_int64( pg_result, row_num, col_num ) \
        strtoll(PQgetvalue(pg_result, row_num, col_num), NULL, 10)

#define dbx_as_datetime( pg_result, dt_ptr, row_num, col_num )                 \
        datetime_from_string(                                                  \
            dt_ptr, PQgetvalue(pg_result, row_num, col_num),DATETIME_NTS,"UTC" \
        )

#define dbx_result_as_string( pg_result, row_num, col_name ) \
        PQgetvalue(pg_result, row_num, PQfnumber(pg_result, col_name))

#define dbx_result_as_integer( pg_result, row_num, col_name ) \
        strtol(\
            PQgetvalue(pg_result, row_num, PQfnumber(pg_result, col_name)),\
            NULL,\
            10\
        )

#define dbx_result_as_int64( pg_result, row_num, col_name ) \
        strtoll(\
            PQgetvalue(pg_result, row_num, PQfnumber(pg_result, col_name)),\
            NULL,\
            10\
        )
#endif
