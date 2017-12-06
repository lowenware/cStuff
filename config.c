#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

/* types -------------------------------------------------------------------- */

struct config_line
{
  int    i_key;
  int    l_key;
  int    i_equal;
  int    i_value;
  int    l_value;
  int    i_comment;
  int    l_comment;
};

typedef struct config_line * config_line_t;

/* path --------------------------------------------------------------------- */

typedef char * config_path_t;

#define PATH_SIZE(x)   ( *((int *)(x-8)) )
#define PATH_LENGTH(x) ( *((int *)(x-4)) )

/* -------------------------------------------------------------------------- */

static config_path_t
config_path_new( char * data, int d_len, int d_size )
{
  config_path_t path = malloc( 4+4+d_size );
  if (path)
  {
    path += 8;
    PATH_SIZE( path )   = d_size;
    PATH_LENGTH( path ) = d_len;
    if (data)
      strncpy( path, data, d_len );
  }
  path[d_len] = 0;

  return path;
}

/* -------------------------------------------------------------------------- */

static void
config_path_free( config_path_t path )
{
  free( path - 8 );
}

/* -------------------------------------------------------------------------- */

static config_status_t
config_path_set_level( config_path_t path, int level )
{
  char * pch = path;
  if (level)
  {
    while( level )
    {
      pch = strchr( pch, '.' );
      if ( pch )
      {
        if (!(--level))
        {
          PATH_LENGTH( path ) = (int)(pch-path);
          break;
        }
        else
          pch++;
      }
      else if ( level == 1 )
      {
        break;
      }
      else
        return CONFIG_INDENT_ERROR;
    }
  }
  else
    PATH_LENGTH( path ) = 0;

  path[ PATH_LENGTH( path ) ] = 0;

  return CONFIG_SUCCESS;
}

/* -------------------------------------------------------------------------- */

static config_status_t
config_path_join( config_path_t * path, const char * key, int k_len )
{
  config_path_t p;

  if( PATH_LENGTH( *path ) + k_len + 2 < PATH_SIZE( *path ) )
  {
    p = config_path_new(
          *path,
          PATH_LENGTH(*path),
          PATH_SIZE(*path) + CONFIG_INITIAL_PATH + k_len
        );

    if (!p)
      return CONFIG_ALLOC_ERROR;


    config_path_free( *path );
    *path = p;
  }
  else
    p = *path;

  if ( PATH_LENGTH( p ) )
  {
    p[ PATH_LENGTH(p) ] = '.';
    PATH_LENGTH(p)++;
  }

  strncpy( &p[ PATH_LENGTH(p) ], key, k_len );
  PATH_LENGTH(p)+=k_len;
  p[ PATH_LENGTH(p) ] = 0;

  return CONFIG_SUCCESS;
}

/* static ------------------------------------------------------------------- */


/* main parsing method, called by config_read
 * @param self config_t object
 * @param line buffer with null-terminated line
 * @param l pointer to config_line_t structure, that will be created and parsed
 * @param stack object of list_t that will be used as a stack for config nodes
 * @result CONFIG_SUCCESS or sconfig_tSyntaxError
 * */
static config_status_t
config_parse_line( char * buffer, config_line_t line, int * offset )
{
  int    last_char = -1;
  char * p    = buffer;

  line->i_key     = -1;
  line->l_key     =  0;
  line->i_equal   = -1;
  line->i_value   = -1;
  line->l_value   =  0;
  line->i_comment = -1;
  line->l_comment =  0;

  while (*p)
  {
    *offset = (int) (p-buffer);

    switch( *p )
    {
      case '=':
      case ':':
        if( line->i_equal == -1 )
        {
          line->i_equal = *offset;
          line->l_key   = last_char - line->i_key + 1;
          if (*p == '=')
            line->i_value = *offset+1;
          last_char = -1;
        }

        break;

      case ';':
        line->i_comment = *offset;
        line->l_comment = strlen(p);
        p += line->l_comment-1;
        break;


      case ' ':
      case '\t':
      case '\n':
        break;

      default:
        if (last_char == -1)
        {
          if (line->i_key == -1)
            line->i_key = *offset;
          else if (line->i_equal != -1 && buffer[ line->i_equal ] == ':')
            return CONFIG_SYNTAX_ERROR;
          else if (line->i_value != -1)
            line->i_value = * offset;
        }

        last_char = *offset;
    }

    p++;
  }


  if (last_char != -1)
  {
    if ( line->i_equal == -1  )
    {
      *offset = last_char;
      return CONFIG_SYNTAX_ERROR;
    }

    if ( line->i_value != -1 )
    {
      line->l_value = last_char - line->i_value + 1;
    }
  }

  return CONFIG_SUCCESS;
}

/* parser ------------------------------------------------------------------- */

config_status_t
config_parse( const char               * filename,
              config_on_get_node_t       on_get_node,
              config_on_get_pair_t       on_get_pair,
              config_on_syntax_error_t   on_syntax_error,
              void                     * u_ptr)
{
  FILE * f = fopen(filename, "r");
  if (!f)
    return CONFIG_FILE_ERROR;

  config_status_t    result = CONFIG_SUCCESS;

  struct config_line line;
  config_path_t      path;

  int                l_num  = 0,
                     level  = 0,
                     l      = 0,
                     indent = 0, 
                     offset;

  char             * buffer;

  if ( (buffer = malloc( CONFIG_INITIAL_BUFFER )) != NULL )
  {
    if ( (path = config_path_new( NULL, 0, CONFIG_INITIAL_PATH )) != NULL )
    {
      /* read file line by line */
      while(fgets(buffer, CONFIG_INITIAL_BUFFER, f))
      {
        l_num++;

        /* parse line into struct config_line */
        result = config_parse_line( buffer, &line, &offset );

        if (result != CONFIG_SUCCESS) /* parser error at @offset character */
        {
          if (on_syntax_error)
          {
            if (on_syntax_error( l_num, offset, buffer, result, u_ptr))
              continue;
          }

          break;
        }

        if (line.i_key == -1) continue; /* skip empty and comments */

        /* indent form line begin, always equal to number of dots in path */
        if (!indent)
          indent = line.i_key;

        l = (indent) ? line.i_key / indent : 0;

        if (l > level)
        {
          if (l - level > 1)
          {
            result = CONFIG_INDENT_ERROR;
            if (on_syntax_error)
            {
              if (on_syntax_error(l_num, line.i_key, buffer, result, u_ptr))
                continue;
            }
            break;
          }
        }


        if ((result = config_path_set_level( path, l )) != CONFIG_SUCCESS)
        {
          if (on_syntax_error)
          {
            if (on_syntax_error(l_num, line.i_key, buffer, result, u_ptr))
              continue;
            break;
          }
        }

        result = config_path_join(&path, &buffer[ line.i_key ], line.l_key);

        if (result != CONFIG_SUCCESS)
        {
          break;
        }

        if (line.i_value == -1) /* node */
        {
          level = l;
          if (on_get_node)
          {
            if ( !on_get_node(l_num, path, u_ptr))
            {
              result = CONFIG_READ_NODE_ERROR;
              break;
            }
          }
        }
        else
        {
          buffer[ line.i_value + line.l_value ] = 0;
          
          if (on_get_pair)
          {
            if (!on_get_pair(
                  l_num, path, &buffer[line.i_value], line.l_value, u_ptr))
            {
              result = CONFIG_READ_PAIR_ERROR;
              break;
            }
          }
        }

      } /* while (fgets(...)) */

      config_path_free( path );
    }
    else
      result = CONFIG_ALLOC_ERROR;

    free( buffer );
  }
  else
    result = CONFIG_ALLOC_ERROR;
  
  fclose(f);

  return result;
}

/* -------------------------------------------------------------------------- */

const char*
config_status_get_text( config_status_t status )
{
  switch(status)
  {
    case CONFIG_ALLOC_ERROR:     return "memory allocation error";
    case CONFIG_FILE_ERROR:      return "file i/o error";
    case CONFIG_INDENT_ERROR:    return "indent error";
    case CONFIG_READ_PAIR_ERROR: return "pair error";
    case CONFIG_READ_NODE_ERROR: return "node error";
    case CONFIG_SYNTAX_ERROR:    return "syntax error";

    case CONFIG_SUCCESS:
      return "success";
  }

  return "unknown";
}

/* -------------------------------------------------------------------------- */
