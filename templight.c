/* - templight.c --------------------------------------------------------------
 *
 * Copyright (c) 2017 Löwenware Ltd (https://lowenware.com)
 *
 * REPOSITORY:
 *   https://github.com/lowenware.com:cStuff.git
 * MAINTAINER:
 *   Elias Löwe <elias@lowenware.com>
 *
 * LICENSE and DISCLAIMER:
 *   All code stored in this repository is designed to solve
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
 * -------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "list.h"
#include "str-utils.h"
#include "str-builder.h"
#include "retcodes.h"

#include "templight.h"

#ifndef TEMPLIGHT_BUFFER_SIZE
#define TEMPLIGHT_BUFFER_SIZE 256
#endif

#define TEMPLIGHT_GREEDY      1
/* types -------------------------------------------------------------------- */

typedef enum
{
  PLAIN_NODE = 1,    /* data is HTML (char *) */
  BLOCK_NODE,        /* data is another template (templight_t *) */
  VAR_NODE,        /* data is data is a value of label (char *) */
  LINK_NODE          /* data is a constant C string (const char *) */
} node_type_t;

/* -------------------------------------------------------------------------- */

struct node
{
  void        *data;
  int          size;
  node_type_t  type;
};

typedef struct node * node_t;

/* -------------------------------------------------------------------------- */

static node_t
node_new(node_type_t type, void * data, int size)
{
  node_t self = malloc(sizeof(struct node));
  if (self)
  {
    self->type = type;
    self->data = data;
    self->size = size;
  }

  return self;
}

/* -------------------------------------------------------------------------- */

static void
node_free(node_t self)
{
  if (self->data)
  {
    switch(self->type)
    {
      case LINK_NODE:
        break;

      case BLOCK_NODE:
        templight_free( (templight_t) self->data );
        break;
      case PLAIN_NODE:
      case VAR_NODE:
        free(self->data);
        break;
    }
  }
  free(self);
}

/* pair helpers ------------------------------------------------------------- */

struct pair
{
  char   *key;
  node_t  node;
};

typedef struct pair * pair_t;

/* -------------------------------------------------------------------------- */

static pair_t
pair_new(const char * key, int k_len, void * node)
{
  pair_t self = malloc(sizeof(struct pair));

  if (self)
  {
    self->node = node;
    if (key)
    {
      if ( (self->key = str_ncopy(key, k_len)) == NULL )
      {
        free(self);
        self=NULL;
      }
    }
    else
      self->key = NULL;
  }
  return self;
}

/* -------------------------------------------------------------------------- */

static void
pair_free(pair_t self)
{
  if(self->key)
    free(self->key);

  free(self);
}

/* templight_t private ------------------------------------------------------ */

struct templight
{
  const char * name;

  list_t       nodes;
  list_t       pairs;
  int          c_length;  /* content length */
  int          flags;
};


/* static functions --------------------------------------------------------- */

/*
static int
_print_error(const char * error, int line)
{
  fprintf(stderr, "[templight] parser error, line #%d: %s\n", line, error);
  return line;
}
*/

/* -------------------------------------------------------------------------- */

static node_t
_append_node(templight_t block, node_type_t n_type, void *n_data, int n_size)
{
  node_t node;

  if (!block->nodes)
  {
    if ( !(block->nodes = list_new(1)) )
      return NULL;
  }

  node = node_new(n_type, n_data, n_size);

  if ( node )
  {
    if (list_append(block->nodes, node) == -1)
    {
      node_free(node);
      return NULL;
    }

    if (n_type == PLAIN_NODE && n_data)
      block->c_length += n_size;
  }

  return node;
}

/* -------------------------------------------------------------------------- */

static pair_t
_append_pair(templight_t block, char * key, node_t node)
{
  pair_t pair;
  
  if (!block->pairs)
  {
    if ( !(block->pairs = list_new(1)) )
      return NULL;
  }

  if ( (pair = pair_new(NULL, 0, node)) != NULL )
  {
    if (list_append(block->pairs, pair) == -1)
    {
      pair_free(pair);
      return NULL;
    }

    pair->key = key; /* key is already allocated by str-builder */
  }


  return pair;
}

/* -------------------------------------------------------------------------- */

templight_t
_new_block(char * block_name, int length, int flags)
{
  templight_t self;
  char * p=&block_name[length-1];

  while(p>block_name)
  {
    if (*p==' ')
    {
      p--;
      length--;
    }
    else
      break;
  }

  if ( (p = str_ncopy(block_name, length)) == NULL)
  {
    return NULL;
  }

  if ( (self = malloc(sizeof(struct templight))) != NULL)
  {
    self->name     = p;
    self->nodes    = NULL;
    self->pairs    = NULL;
    self->c_length = 0;
    self->flags    = flags;
  }
  else
    free(p);

  return self;
}

/* -------------------------------------------------------------------------- */

static int
_parse(FILE * fd, list_t stack)
{
  /* const */
  enum {
    MACRO_SEEK,
    MACRO_VAR,
    MACRO_BEGIN,
    MACRO_END
  };

  const char c_begin_open[]  = "{:begin ",
             c_begin_close[] = "}",
             c_var_open[]    = "{:var ",
             c_var_close[]   = "}",
             c_end[]         = "{:end}";

  /* vars */

  int     b_size = TEMPLIGHT_BUFFER_SIZE,
          d_len = 0,
          l, i,
          cursor,
          mode = MACRO_SEEK,
          result;

  char  * buffer;

  str_builder_t s_bldr;

  templight_t    pr = (templight_t) list_index(stack, stack->count-1),
                 bl;

  pair_t    pair;
  node_t    node;

  /* allocate resources */

  if ( !(buffer = malloc( b_size )) )
    RAISE( CSTUFF_MALLOC_ERROR, finally );

  if ( !(s_bldr = str_builder_new( 0 )) )
    RAISE( CSTUFF_MALLOC_ERROR, release_buffer );

  /* read to buffer */
  while( (l = fread(&buffer[d_len], 1, b_size - d_len - 1, fd)) > 0 )
  {
    /* printf("%d = fread(&buffer[%d], 1, %d-%d-1, fd)\n", l, d_len, b_size, d_len); */
    d_len+=l;

    buffer[d_len] = 0;
    cursor = 0;

    for (i=0; i<d_len; i++)
    {
      if (mode == MACRO_SEEK)
      {
        if (strncmp(&buffer[i], c_begin_open, sizeof(c_begin_open)-1 ) == 0)
        {
          mode = MACRO_BEGIN;
          l = sizeof(c_begin_open)-1;
        }
        else if (strncmp(&buffer[i], c_var_open, sizeof(c_var_open)-1)==0)
        {
          mode = MACRO_VAR;
          l = sizeof(c_var_open)-1;
        }
        else if (strncmp(&buffer[i], c_end, sizeof(c_end)-1 ) == 0)
        {
          if (stack->count == 1)
          {
            /* return _print_error("closing of non-opened block", line); */
            return CSTUFF_PARSE_ERROR;
          }

          /* close current plain node */
          if (str_builder_append_chars(s_bldr, &buffer[ cursor ], i-cursor)==-1)
            goto e_malloc;
          

          if ( !(_append_node(pr, PLAIN_NODE, s_bldr->c_str, s_bldr->length)) )
            goto e_malloc;

          memset( s_bldr, 0, sizeof( struct str_builder ) );

          list_remove_index(stack, stack->count-1);
          pr = list_index(stack, stack->count-1);

          i += sizeof(c_end)-1;
        }
        else
        {
          l = d_len - i;
          if (
               (l < sizeof( c_var_open ) -1 ) ||
               (l < sizeof( c_begin_open ) -1 ) ||
               (l < sizeof( c_end ) -1 )
             )
          {
            break;
          }

          continue;
        }

        if (mode != MACRO_SEEK)
        {
          /* close current plain node */
          if (str_builder_append_chars(s_bldr, &buffer[ cursor ], i-cursor)==-1)
            goto e_malloc;


          if ( !(_append_node(pr, PLAIN_NODE, s_bldr->c_str, s_bldr->length)) )
            goto e_malloc;

          memset( s_bldr, 0, sizeof( struct str_builder ) );

          i+=l; /* add length of macro */
        }

        /* move cursor */
        cursor = i;
      }

      if ( mode == MACRO_BEGIN )
      {
        if (strncmp(&buffer[i], c_begin_close, sizeof(c_begin_close)-1)==0)
        {
          if (str_builder_append_chars(s_bldr, &buffer[ cursor ], i-cursor)==-1)
            goto e_malloc;


          if ( !(bl = _new_block( s_bldr->c_str, s_bldr->length, pr->flags )) )
            goto e_malloc;

          if ( !(node=_append_node( pr, BLOCK_NODE, (void *) bl, 0)) )
          {
            templight_free(bl);
            goto e_malloc;
          }

          if ( !(pair=_append_pair( pr, s_bldr->c_str, node)) )
            goto e_malloc;
          
          memset( s_bldr, 0, sizeof( struct str_builder ) );

          if ( list_append(stack, (void*) bl) == -1)
            goto e_malloc;

          pr = bl;

          i += (sizeof(c_begin_close)-1);
        }
        else if ( d_len-i < sizeof(c_begin_close)-1 )
        {
          break;  /* load more chars */
        }
        else
          continue;
      }
      else if ( mode == MACRO_VAR )
      {
        if (strncmp(&buffer[i], c_var_close, sizeof(c_var_close)-1)==0)
        {
          if (str_builder_append_chars(s_bldr, &buffer[ cursor ], i-cursor)==-1)
            goto e_malloc;

          if ( !(node=_append_node(pr, VAR_NODE, NULL, 0)) )
            goto e_malloc;

          if ( !(_append_pair(pr, s_bldr->c_str, node)) )
            goto e_malloc;

          memset( s_bldr, 0, sizeof( struct str_builder ) );

          i += (sizeof(c_var_close)-1);
        }
        else if ( d_len-i < sizeof(c_var_close)-1 )
        {
          break;  /* load more chars */
        }
        else
          continue;
      }

      mode = MACRO_SEEK;
      cursor = i;

    } /* end for */

    if ( cursor < i )
    {
      if (str_builder_append_chars(s_bldr, &buffer[ cursor ], i-cursor)==-1)
        goto e_malloc;

      /* printf("PLAIN 2: %s\n", s_bldr->c_str);
      if ( !(_append_node(pr, PLAIN_NODE, s_bldr->c_str, s_bldr->length)) )
        goto e_malloc; 

      memset( s_bldr, 0, sizeof( struct str_builder ) );
      */

      cursor=i;
    }

    if ((d_len -= i) < 0 )
    {
      d_len = 0;
    }
    else if (d_len > 0)
    {
      mode = MACRO_SEEK;
      memmove( buffer, &buffer[i], d_len );
      continue;
    }


  } /* end while */

  /* add last plain node */
  if (d_len)
  {
     if (str_builder_append_chars(s_bldr, &buffer[ cursor ], d_len)==-1)
        goto e_malloc;

     if ( !(_append_node(pr, PLAIN_NODE, s_bldr->c_str, s_bldr->length)) )
        goto e_malloc;

     memset( s_bldr, 0, sizeof( struct str_builder ) );

  }

  /* check stack */
  if (stack->count > 1)
  {
     /*fprintf(stderr, "[templight] block is not closed %s\n", 
     ((templight_t) list_index(stack, stack->count-1))->name); */
     result = CSTUFF_PARSE_ERROR;
     goto e_malloc;
  }

  result = CSTUFF_SUCCESS;
  goto release;

  /* release resources  */
e_malloc:
  result = CSTUFF_MALLOC_ERROR;

release:
  str_builder_free( s_bldr, 1 );

release_buffer:
  free(buffer);

finally:
  return result;
}

/*
int
bak_parse_line(list_t stack, char * b, int line)
{
  templight_t    block, parent;
  char          *begin;
  char          *end;
  int            l;
  node_t         node;
  pair_t         pair;

  / * handle the end of block * /
  begin = strstr(b, "{:end}");
  if (begin)
  {
    if (stack->count == 1)
    {
      / * return _print_error("closing of non-opened block", line); * /
      return CSTUFF_PARSE_ERROR;
    }

    list_remove_index(stack, stack->count-1);
    return 0;
  }

  / * handle the begining of block * /
  begin = strstr(b, "{:begin ");
  if (begin) 
  {
    begin += 8;
    end = strstr(begin, "}");
    if (end && end > begin)
    {
      block = _new_block( begin, (int) (end-begin) );
      if (block)
      {
        parent = (templight_t) list_index(stack, stack->count-1);
        node = _append_node( parent, BLOCK_NODE, (void *) block, 0 );

        if (node)
        {
          pair = _append_pair( parent, begin, (int) (end-begin), node);

          if (pair)
          {
            if ( list_append(stack, (void*) block) != -1)
            {
              return 0;
            }
          }
        }
      }
      return line;
    }
    else
      return CSTUFF_PARSE_ERROR; / * _print_error("bad block name", line); * /
  }

  / * Parse labels and plain text * /
  block = (stack->count) ? list_index(stack, stack->count-1) : NULL;
  end = b;
  while ( (begin = strstr(end, "{:var ")) )
  {
    / * some text before label * /
    if (begin > end) 
    {
      l = (int) (begin-end);
      _append_node( block, PLAIN_NODE, str_ncopy(end, l), l);
    }

    / * saving label * /
    begin+=6;
    end = strstr(begin, "}");
    if(end)
    {
      _append_pair(
         block, begin, (int)(end-begin), _append_node(block, VAR_NODE, NULL, 0)
      );

      end++;
    }
    else
      return _print_error("bad label name", line);
  }

  / * plain text, even after label * /
  l = strlen(end);
  if (l>0) _append_node( block, PLAIN_NODE, str_ncopy(end, l), l);

  return 0; // SUCCESS
}
*/

/* public functions --------------------------------------------------------- */

int
templight_new(templight_t * self, const char * name, const char * root)
{
  list_t        stack;
  FILE         *fd;
  char         *fpath;                         /* file path */
  int           result = CSTUFF_SUCCESS;

  if ( !(fpath = str_printf("%s/%s.tpl.html", root, name)) )
    return CSTUFF_MALLOC_ERROR;

  if ( !(fd = fopen(fpath, "r")) )
    RAISE(CSTUFF_SYSCALL_ERROR, release_fpath);

  if ( !(stack = list_new(4)) ) /* stack for block tree */
    RAISE(CSTUFF_MALLOC_ERROR, release_file);

  /* init object */
  if ( !(*self = malloc(sizeof(struct templight))) )
    RAISE(CSTUFF_MALLOC_ERROR, release_stack);

  if ( ! ((*self)->name = str_copy(name)) )
    RAISE( CSTUFF_MALLOC_ERROR, except );

  (*self)->nodes    = NULL;
  (*self)->pairs    = NULL;
  (*self)->c_length = 0;
  (*self)->flags    = TEMPLIGHT_GREEDY;

  if (list_append(stack, *self) == -1)
    RAISE( CSTUFF_MALLOC_ERROR, except );

  result = _parse( fd, stack );

  if (result == CSTUFF_SUCCESS)
    goto finally;

except:
  templight_free(*self);
  *self = NULL;

finally:

release_stack:
  list_free(stack, NULL);

release_file:
  fclose(fd);

release_fpath:
  free( fpath );

  return result;
}

/* -------------------------------------------------------------------------- */

void
templight_free(templight_t self)
{
  if (self)
  {
    if (self->name) free( (char*)self->name);

    if (self->pairs)
      list_free(self->pairs, (list_destructor_t) pair_free);

    if (self->nodes)
      list_free(self->nodes, (list_destructor_t) node_free);

    free(self);
  }
}

/* -------------------------------------------------------------------------- */

void
templight_set_greedy( templight_t self, int greedy )
{
  if (greedy)
    self->flags |= TEMPLIGHT_GREEDY;
  else if (self->flags & TEMPLIGHT_GREEDY)
    self->flags ^= TEMPLIGHT_GREEDY;
}

/* -------------------------------------------------------------------------- */

int
templight_get_greedy( templight_t self )
{
  return (self->flags & TEMPLIGHT_GREEDY);
}


/* -------------------------------------------------------------------------- */

const char *
templight_get_name(templight_t self)
{
  return self->name;
}

/* -------------------------------------------------------------------------- */

int
templight_clone(templight_t self, templight_t * clone)
{
  int  iN,      /* nodes index */
       iP  = 0, /* pairs index */
       result;

  void * ptr;

  node_t n;
  pair_t p;

  if ( (*clone = calloc(1, sizeof(struct templight))) == NULL)
    goto e_malloc;

  (*clone)->flags = self->flags;

  if ( ((*clone)->name = str_copy(self->name)) == NULL )
    goto e_malloc;

  if (self->nodes)
  {
    if ( ((*clone)->nodes = list_new(self->nodes->count)) == NULL)
      goto e_malloc;


    if ( self->pairs && ((*clone)->pairs = list_new(self->pairs->count))==NULL)
      goto e_malloc;

    for (iN=0; iN<self->nodes->count; iN++)
    {
      n = list_index(self->nodes, iN);

      switch (n->type)
      {
        case PLAIN_NODE:
        case LINK_NODE:
          if ( (n = node_new(LINK_NODE, n->data, n->size)) == NULL )
            goto e_malloc;

          if ( list_append( (*clone)->nodes, n ) == -1 )
            goto e_malloc;

          continue;

        case VAR_NODE:
          if ( n->data )
          {
            if ( (ptr = str_copy((const char *) n->data))== NULL )
              goto e_malloc;
          }
          else
            ptr = NULL;

          if ( (n = node_new(VAR_NODE, ptr, n->size)) == NULL )
            goto e_malloc;

          if (list_append((*clone)->nodes, n) == -1)
            goto e_malloc;

          p = (pair_t) list_index(self->pairs, iP++);
          if ( (p = pair_new(p->key, strlen(p->key), n)) == NULL )
            goto e_malloc;

          if (list_append((*clone)->pairs, p) == -1)
            goto e_malloc;

          continue;

        case BLOCK_NODE:
          result = templight_clone((templight_t) n->data, (templight_t *) &ptr);
          if ( result != CSTUFF_SUCCESS )
            goto except;

          if ( (n = node_new(BLOCK_NODE, ptr, n->size)) == NULL )
            goto e_malloc;

          if (n->size != -1)
          {
            p = (pair_t) list_index(self->pairs, iP++);
            if ( (p = pair_new(p->key, strlen(p->key), n)) == NULL )
              goto e_malloc;

            if (list_append((*clone)->pairs, p) == -1)
              goto e_malloc;
          }

          if ( list_append((*clone)->nodes, n) == -1)
            goto e_malloc;

          continue;
      }
    }
  }

  result = CSTUFF_SUCCESS;
  goto finally;

e_malloc:
  result = CSTUFF_MALLOC_ERROR;

except:
  if (*clone)
    templight_free( *clone );
  *clone = NULL;
finally:
  return result;
}

/* -------------------------------------------------------------------------- */

int
templight_new_block(templight_t self, templight_t * block, const char * label)
{
  int         result, 
              i;
  pair_t      p;
  node_t      n;

  if (!self->pairs)
    return CSTUFF_NULL_OBJECT;

  for(i=0; i<self->pairs->count; i++)
  {
    p = (pair_t) list_index(self->pairs, i);

    if (p->node->type != BLOCK_NODE || strcmp( p->key, label)) continue;
    /* pair found */

    /* block node found */
    if (p->node->data)
    {
      p->node->size++;

      result = templight_clone( (templight_t ) p->node->data, block );
      if (result != CSTUFF_SUCCESS)
        goto except;

      (*block)->flags = self->flags;

      /* find insert position */
      for (i=0; i<self->nodes->count; i++)
      {
        if (list_index(self->nodes, i) == p->node)
        {
          if ( (n = node_new(BLOCK_NODE, *block, -1)) == NULL )
            goto e_malloc;

          if ( (list_insert( self->nodes, n, i+p->node->size ) == -1) )
          {
            node_free( n );
            goto e_malloc;
          }

          result = CSTUFF_SUCCESS;
          goto finally;
        }
      }
    }

    break;
  }

  result = CSTUFF_NOT_FOUND;
  goto finally;

e_malloc:
  result = CSTUFF_MALLOC_ERROR;

except:
  templight_free(*block);
  *block = NULL;

finally:
  return result;
}


/* -------------------------------------------------------------------------- */

static int
_set_value(templight_t self, const char *var_name, char *value)
{
  int       i, l, result;
  pair_t    p;
  char    * ptr;

  result = 0;

    
  if (!self->pairs)
    return result;


  l   = strlen(value),
  ptr = value;

  for(i=0; i<self->pairs->count; i++)
  {
    p = (pair_t) list_index(self->pairs, i);
    if ( p->node->type == VAR_NODE && strcmp(p->key, var_name)==0 )
    {
      if (result)
      {
        if ( (ptr = str_ncopy(value, l))==NULL )
          goto except;
      }

      if ( p->node->data )
      {
        self->c_length -= strlen( (const char*) p->node->data );
        free( p->node->data );
      }

      p->node->data   = ptr;
      p->node->size   = l;
      self->c_length += l;

      ptr = NULL;

      result++;

      if ( !(self->flags & TEMPLIGHT_GREEDY) )
        goto finally;
    }
  }

  goto finally;

except:
  result = -1;

finally:
  if (ptr) free(ptr);

  return result;
}

/* -------------------------------------------------------------------------- */

int
templight_set_printf(templight_t self, const char * var_name, 
                                       const char * format, ...)
{
  va_list list;
  va_start(list, format);
  int result = templight_set_vprintf(self, var_name, format, list);
  va_end(list);
  return result;
}

/* -------------------------------------------------------------------------- */

int
templight_set_vprintf(templight_t self, const char * var_name, 
                                        const char * format,
                                        va_list      list)
{
  char * value = str_vprintf(format, list);

  return (value) ? _set_value(self, var_name, value) : -1;
}

/* -------------------------------------------------------------------------- */

int
templight_set_string(templight_t self, const char * var_name,
                                       const char * value )
{
  char * v = str_copy(value ? value : "");

  return (v) ? _set_value(self, var_name, v) : -1;
}

/* -------------------------------------------------------------------------- */

int
templight_set_integer(templight_t self, const char * var_name,
                                        const int    value)
{
  char * v = str_printf("%d", value);
  return v ? _set_value(self, var_name, v) : -1;
}

/* -------------------------------------------------------------------------- */

int
templight_set_float(templight_t self,   const char   * var_name,
                                        const double   value)
{
  char * v = str_printf("%f", value);
  return v ? _set_value(self, var_name, v) : -1;
}

/* -------------------------------------------------------------------------- */

int
templight_set_block(templight_t self, const char * var_name,
                                      templight_t  value)
{
  int      i, r, result;
  pair_t   p;

  result = 0;

  for(i=0; i<self->pairs->count; i++)
  {
    p = (pair_t) list_index(self->pairs, i);

    if ( strcmp( p->key, var_name)==0 )
    {
      if (p->node->type == VAR_NODE)
      {
        if (p->node->data)
        {
          self->c_length -= strlen( (const char *)p->node->data);
          free(p->node->data);
        }
        p->node->type = BLOCK_NODE;
      }
      else if (p->node->type == BLOCK_NODE && p->node->data)
        templight_free((templight_t)p->node->data);

      p->node->size = 0;

      if (result)
      {
        r = templight_clone(value, (templight_t *) &p->node->data);

        if ( r != CSTUFF_SUCCESS )
          goto except;
      }
      else
        p->node->data = value;

      result++;

      if ( !(self->flags & TEMPLIGHT_GREEDY) )
        goto finally;

    }
  }

  goto finally;

except:
  result = -1;
finally:
  return result;
}

/* -------------------------------------------------------------------------- */
#ifdef CSTUFF_TEMPLIGHT_WITH_TO_AISL_STREAM

int
templight_to_aisl_stream(templight_t self, aisl_stream_t s)
{
  int    i = 0,
         result = CSTUFF_SUCCESS;

  node_t n;

  if (!self->nodes)
    goto finally;


  for ( i=0; i<self->nodes->count; i++)
  {
    if ( (n = list_index(self->nodes, i))==NULL)
      continue;

    switch (n->type)
    {
      case PLAIN_NODE:
      case LINK_NODE:
        if ( aisl_write(s, (const char*) n->data, n->size) == -1)
          goto e_extcall;

        break;

      case VAR_NODE:
        if (n->data)
          if ( aisl_write(s, (const char *) n->data, n->size) == -1 )
            goto e_extcall;

        break;

      case BLOCK_NODE:
        if ( n->size == -1)
        {
          result = templight_to_aisl_stream( ((templight_t) n->data), s );
          if ( result != CSTUFF_SUCCESS )
            goto except;
        }
        break;
    }
  }

  goto finally;

e_extcall:
  result = CSTUFF_EXTCALL_ERROR;
except:

finally:
  return result;
}

#endif
/* -------------------------------------------------------------------------- */

int
templight_get_content_length(templight_t self)
{
  int c_length = self->c_length,
      i;

  node_t   node;

  if (self->nodes)
  {
    for (i=0; i<self->nodes->count; i++)
    {
      if ((node = list_index(self->nodes, i))==NULL) continue;

      if ( (node->type == BLOCK_NODE) && (node->size == -1) && node->data)
        c_length += templight_get_content_length( (templight_t) node->data );
    }
  }

  return c_length;
}

/* -------------------------------------------------------------------------- */

int
templight_to_fstream(templight_t self, FILE * fstream)
{
  int    i = 0, l,
         result = CSTUFF_SUCCESS;

  node_t n;


  if (!self->nodes)
    goto finally;

  for ( i=0; i<self->nodes->count; i++)
  {
    if ( (n = list_index(self->nodes, i))==NULL)
      continue;

    switch (n->type)
    {
      case PLAIN_NODE:
      case LINK_NODE:
      case VAR_NODE:
        l = fwrite((const char*) n->data, 1, n->size, fstream);
        if ( l != n->size)
          goto e_extcall;
        break;

      case BLOCK_NODE:
        if ( n->size == -1)
        {
          result = templight_to_fstream( ((templight_t) n->data), fstream );
          if ( result != CSTUFF_SUCCESS )
            goto except;
        }
        break;
    }
  }

  goto finally;

e_extcall:
  result = CSTUFF_EXTCALL_ERROR;
except:

finally:
  return result;
}

