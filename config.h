/* config.h
 *
 * # I/O module for Löwenware config format
 *
 * ## Syntax
 *
 * In general syntax is very similar to Python style, but even less strict.
 *
 * 1. Configuration always has tree structure with unnamed root node.
 * 2. Named nodes must be defined with following colon sign.
 * 3. Parameters and values are splited with = sign.
 * 4. Everything written after semicolon sign is a comment.
 * 5. Indents define nesting level
 *
 * ## Example
 *
 * ; + BEGIN EXAMPLE CONFIG ---------------------------------------------------
 * 
 * name = default config
 * file = lw/config.h
 * example:                           ; this is node
 *   indent = 4                       ; indent for this node is 2 spaces
 *   aim    = demonstrate config usage
 *   syntax = strict
 *   speed  = fast
 *
 * ; + END EXAMPLE CONFIG ---------------------------------------------------
 *
 * */

#ifndef _CSTUFF_CONFIG_H_
#define _CSTUFF_CONFIG_H_

#include <stdbool.h>

/* macro settings ----------------------------------------------------------- */

#ifndef CONFIG_INITIAL_PATH
#define CONFIG_INITIAL_PATH 32
#endif

#ifndef CONFIG_INITIAL_BUFFER
#define CONFIG_INITIAL_BUFFER 82
#endif

/* macro helpres ------------------------------------------------------------ */

#define CONFIG_STRING( NAME, PATH, D_VALUE ) \
  const char NAME##Path[] = PATH; \
  const char NAME##Default[] = D_VALUE; \
  char * NAME = NULL;

#define CONFIG_DEFINE( NAME, PATH, D_VALUE, TYPE ) \
  const char NAME##Path[] = PATH; \
  const TYPE NAME##Default = D_VALUE; \
  TYPE NAME = D_VALUE;

#define CONFIG_SET_DEFAULT( NAME ) \
  NAME = (__typeof(NAME) ) NAME##Default;

#define CONFIG_FREE( NAME ) \
  if ( NAME && ((void*) NAME != (void*) NAME##Default)){ \
    free( NAME ); \
    NAME = NULL; \
  }

/* -------------------------------------------------------------------------- */

#ifdef DEBUG
  #define CONFIG_LOG( ... ) fprintf( stdout, __VA_ARGS__ )
#else
  #define CONFIG_LOG( ... ) log_state( __VA_ARGS__ )
#endif

/* -------------------------------------------------------------------------- */

typedef enum {

  CONFIG_ALLOC_ERROR     = -6,
  CONFIG_FILE_ERROR      = -5,
  CONFIG_INDENT_ERROR    = -4,
  CONFIG_READ_PAIR_ERROR = -3,
  CONFIG_READ_NODE_ERROR = -2,
  CONFIG_SYNTAX_ERROR    = -1,

  CONFIG_SUCCESS         = 0

} config_status_t;

/* -------------------------------------------------------------------------- */

/* This feature is an ideal solution for configuration importing into 
 * application due to its simplicity and low resource usage.
 * */

/* Function being called on reading of each configuration node. Could be useful
 * in case if you need to import several nodes with same name, i.e. 
 * configuration for number of instances or something like that
 *
 * @line   : if not null, actual number of node line will be wrtiten by this 
 *           pointer
 * @node   : text representation of node
 * @result : true if parser can continue, false if must stop
 * */
typedef bool
(* config_on_get_node_t)( int          line_number,
                          const char * node,
                          void       * u_ptr );

/* Function being called on parameter pair (key => value) import.
 *
 * @line   : if not null, actual number of node line will be wrtiten by this 
 *           pointer
 * @key    : full configuration key, including parent nodes splitted by dot
 * @value  : always null-terminated parameter value, can not be NULL
 * @v_len  : length of value, being passed because it is known, to avoid
 *           unnecessary further strlen calls
 * @result : true if parser can continue, false if must stop
 * */
typedef bool
(* config_on_get_pair_t)( int          line_number,
                          const char * key,
                          const char * value,
                          int          v_len,
                          void       * u_ptr );

/* Function being called on synatx error detection
 *
 * @line      : if not null, actual number of node line will be wrtiten by this 
 *              pointer
 * @row_text  : row text caused an error 
 * @err_code  : error code
 * @character : character offset in row
 * @result    : true if parser can continue, false if must stop
 * */
typedef bool
(* config_on_syntax_error_t)( int               line_number,
                              int               char_number,
                              const char      * line_text,
                              config_status_t   err_code,
                              void            * u_ptr);

/* Reads file defined by filename, parses its syntax and call functions
 * passed as arguments.
 * @filename        : path to configuration file
 * @on_read_node    : read node callback
 * @on_read_node    : read pair callback
 * @on_syntax_error : syntax error callback
 * @result          : parser result
 * */
config_status_t
config_parse( const char                * filename,
              config_on_get_node_t        on_get_node,
              config_on_get_pair_t        on_get_pair,
              config_on_syntax_error_t    on_syntax_error,
              void                      * u_ptr );


/* -------------------------------------------------------------------------- */

const char*
config_status_get_text( config_status_t status );

/* -------------------------------------------------------------------------- */

#endif
