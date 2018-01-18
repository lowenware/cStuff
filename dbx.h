#ifndef _CSTUFF_DBX_H_
#define _CSTUFF_DBX_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <libpq-fe.h>

#include "retcodes.h"

/* type --------------------------------------------------------------------- */

typedef enum
{
  DBX_INT,
  DBX_UINT,
  DBX_INT64,
  DBX_UINT64,

  DBX_STRING,
  DBX_CONSTANT,
  DBX_MD5_HASH,
  DBX_STATEMENT,

  DBX_BOOLEAN

} dbx_param_t;

typedef bool /* true - free result, false - keep result */
(*dbx_on_result_t)( PGresult   * result,
                    int          res_i,
                    void       * u_data);

typedef void
(*dbx_on_error_t)(  const char * e_message,
                    int          res_i,
                    void       * u_data,
                    const char * e_sql );

#define dbx_escape(x) PQescapeLiteral(dbxConn, x, strlen(x))
#define dbx_free(x)   PQfreemem(x)

/* init subsystem ----------------------------------------------------------- */

cstuff_retcode_t
dbx_init( const char * username,
          const char * password,
          const char * database,
          const char * hostname,
          int          port,
          int          connections );

/* release used resources --------------------------------------------------- */

void
dbx_release();

/* refresh subsystem data --------------------------------------------------- */

cstuff_retcode_t
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

#define dbx_as_bool( pg_result, row_num, col_num ) \
       ((PQgetvalue(pg_result, row_num, col_num)[0]=='t') ? true : false)

#define dbx_as_integer( pg_result, row_num, col_num ) \
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
