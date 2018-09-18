/* dbx.h : header file of asynchronous PostgreSQL Database Engine
 * */
#ifndef _CSTUFF_DBX_H_
#define _CSTUFF_DBX_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <libpq-fe.h>

#include "retcodes.h"

/* -------------------------------------------------------------------------- */

typedef enum
{
  DBX_INT32,
  DBX_UINT32,
  DBX_INT64,
  DBX_UINT64,

  DBX_FLOAT,
  DBX_TIMESTAMP,

  DBX_STRING,
  DBX_CONSTANT,
  DBX_MD5_HASH,
  DBX_STATEMENT,

  DBX_BOOLEAN

} dbx_param_t;

/* -------------------------------------------------------------------------- */

#define DBX_FLAG_RECONNECT   (1<<0)

#define DBX_FLAG_FREE_SQL    (1<<0)
#define DBX_FLAG_TRANSACTION (1<<1)

/* -------------------------------------------------------------------------- */

typedef bool /* true - free result, false - keep result */
(*dbx_on_result_t)( PGresult   * result,
                    int          res_i,
                    void       * u_data);

/* -------------------------------------------------------------------------- */

typedef void
(*dbx_on_error_t)(  const char * e_message,
                    int          res_i,
                    void       * u_data,
                    const char * e_sql );

/* -------------------------------------------------------------------------- */

#define dbx_escape(x) PQescapeLiteral(dbxConn, x, strlen(x))

#define dbx_free(x)   PQfreemem(x)

/* -------------------------------------------------------------------------- */

cstuff_retcode_t
dbx_init( const char * username,
          const char * password,
          const char * database,
          const char * hostname,
          int          port,
          int          connections );

/* -------------------------------------------------------------------------- */

void
dbx_release();

/* -------------------------------------------------------------------------- */

cstuff_retcode_t
dbx_touch();

/* -------------------------------------------------------------------------- */

char *
dbx_sql_format(const char * sql_format, int count, ...);

/* -------------------------------------------------------------------------- */

char *
dbx_sql_vformat(const char * sql_format, int count, va_list args );


/* -------------------------------------------------------------------------- */

uint64_t
dbx_query_format( const char      * sql_format,
                  dbx_on_result_t   on_result,
                  dbx_on_error_t    on_error,
                  void            * u_data,
                  int               p_count,
                  /* dbx_param_t       param_type,
                   * __TYPE__          param,  */
                   ... );

/* -------------------------------------------------------------------------- */

uint64_t
dbx_query_const( const char      * sql,
                 dbx_on_result_t   on_result,
                 dbx_on_error_t    on_error,
                 void            * u_data );

/* -------------------------------------------------------------------------- */

uint64_t
dbx_query_transaction( const char      * sql,
                       dbx_on_result_t   on_result,
                       dbx_on_error_t    on_error,
                       void            * u_data );

/* -------------------------------------------------------------------------- */


cstuff_retcode_t
dbx_cancel(uint64_t id);

/* -------------------------------------------------------------------------- */

const char *
dbx_get_error();

/* -------------------------------------------------------------------------- */

int
dbx_ready_connections_count();

/* -------------------------------------------------------------------------- */

#define dbx_as_string( pg_result, row_num, col_num ) \
        PQgetvalue(pg_result, row_num, col_num)

/* -------------------------------------------------------------------------- */

#define dbx_as_bool( pg_result, row_num, col_num ) \
       ((PQgetvalue(pg_result, row_num, col_num)[0]=='t') ? true : false)

/* -------------------------------------------------------------------------- */

#define dbx_as_integer( pg_result, row_num, col_num ) \
        strtoll(PQgetvalue(pg_result, row_num, col_num), NULL, 10)

/* -------------------------------------------------------------------------- */

#define dbx_as_datetime( pg_result, dt_ptr, row_num, col_num )                 \
        datetime_from_string(                                                  \
            dt_ptr, PQgetvalue(pg_result, row_num, col_num),DATETIME_NTS,"UTC" \
        )

/* -------------------------------------------------------------------------- */

int
dbx_as_timestamp( PGresult * result, int row_num, int col_num, time_t * p_ts );

/* -------------------------------------------------------------------------- */

#endif
