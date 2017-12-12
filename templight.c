#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "list.h"
#include "str-utils.h"

#include "templight.h"

#ifndef TEMPLIGHT_BUFFER_SIZE
#define TEMPLIGHT_BUFFER_SIZE 256
#endif

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
  if (!self) return;

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
    if ( (self->key  = str_ncopy(key, k_len)) == NULL )
    {
      free(self);
      self=NULL;
    }
  }
  return self;
}

/* -------------------------------------------------------------------------- */

static void
pair_free(pair_t self)
{
  if (!self) return;
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
};


/* static functions --------------------------------------------------------- */

static int
_print_error(const char * error, int line)
{
  fprintf(stderr, "[templight] parser error, line #%d: %s\n", line, error);
  return line;
}

/* -------------------------------------------------------------------------- */

static node_t
_append_node(templight_t block, node_type_t n_type, void *n_data, int n_size)
{
  node_t node;

  if (!block->nodes)
    if ( (block->nodes = list_new(1))==NULL ) return NULL;

  if ( (node = node_new(n_type, n_data, n_size)) != NULL);
  {
    list_append(block->nodes, node);

    if (n_type == PLAIN_NODE && n_data)
      block->c_length += n_size;
  }

  return node;
}

/* -------------------------------------------------------------------------- */

static pair_t
_append_pair(templight_t block, const char * key, int length, node_t node)
{
  pair_t pair;
  
  if (!block->pairs)
    if ( (block->pairs = list_new(1)) == NULL) return NULL;

  if ( (pair = pair_new(key, length, node)) != NULL )
  {
    list_append(block->pairs, pair);
  }

  return pair;
}

/* -------------------------------------------------------------------------- */

templight_t
_new_block(char * block_name, int length)
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
  }

  return self;
}

/* -------------------------------------------------------------------------- */

static int
_parse_line(list_t stack, char * b, int line)
{
  templight_t    block;
  char          *begin;
  char          *end;
  int            l;

  /* handle the end of block */
  begin = strstr(b, "{:end}");
  if (begin)
  {
    if (stack->count == 1)
      return _print_error("closing of non-opened block", line);

    list_remove_index(stack, stack->count-1);
    return 0;
  }

  /* handle the begining of block */
  begin = strstr(b, "{:begin ");
  if (begin) 
  {
    begin += 8;
    end = strstr(begin, "}");
    if (end && end > begin)
    {
      block = _new_block( begin, (int) (end-begin) );

      _append_pair(
        ((templight_t) list_index(stack, stack->count-1)),
        begin, 
        (int) (end-begin), 
        _append_node(
          ((templight_t) list_index(stack, stack->count-1)),
          BLOCK_NODE,
          (void *) block,
          0
        )
      );
      list_append(stack, (void*) block);
      return 0;
    }
    else
      return _print_error("bad block name", line);
  }

  /* Parse labels and plain text */
  block = (stack->count) ? list_index(stack, stack->count-1) : NULL;
  end = b;
  while ( (begin = strstr(end, "{:var ")) )
  {
    /* some text before label */
    if (begin > end) 
    {
      l = (int) (begin-end);
      _append_node( block, PLAIN_NODE, str_ncopy(end, l), l);
    }

    /* saving label */
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

  /* plain text, even after label */
  l = strlen(end);
  if (l>0) _append_node( block, PLAIN_NODE, str_ncopy(end, l), l);

  return 0; // SUCCESS
}

/* public functions --------------------------------------------------------- */

templight_t
templight_new(const char * name, const char * root)
{
  if (!name || !root) return NULL;

  templight_t   self;
  list_t        stack;
  FILE         *fd;
  char          b[TEMPLIGHT_BUFFER_SIZE];   /* buffer */
  char         *fp;                         /* file path */
  int           l;

  l = strlen(name);

  fp = str_printf("%s/%s.tpl.html", root, name);

  fd = fopen(fp, "r");

  if (!fd)
  {
    fprintf(stderr, "[templight] could not open template file (%s): %s\n", 
        fp, strerror(errno));
    free (fp);
    return NULL;
  }

  /* init object */
  self = malloc(sizeof(struct templight));

  self->name     = str_copy(name);
  self->nodes    = NULL;
  self->pairs    = NULL;
  self->c_length = 0;

  stack = list_new(4); /* stack to control block hierarchy */
  list_append(stack, self);

  l=1; /* line number */

  while(fgets(b, TEMPLIGHT_BUFFER_SIZE-1, fd))
  {
    b[TEMPLIGHT_BUFFER_SIZE-1] = 0;
    if (_parse_line(stack, b, l))
    {
      /* exit on parse error */
      templight_free(self);
      self = NULL;
      break;
    }
    l++;
  }

  if (stack->count > 1)
  {
    fprintf(stderr, "[templight] block is not closed %s\n", 
        ((templight_t) list_index(stack, stack->count-1))->name);

    templight_free(self);
    self = NULL;
  }

  list_free(stack, NULL);

  fclose(fd);
  free (fp);


  return self;
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

const char *
templight_get_name(templight_t self)
{
  return (self) ? self->name : "";
}

/* -------------------------------------------------------------------------- */

templight_t
templight_clone(templight_t self)
{
  if (!self) return NULL;

  int  iN, /* nodes index */
       iP = 0; /* pairs index */

  node_t n;
  pair_t p;
  
  templight_t clone = malloc(sizeof(struct templight));

  clone->name     = str_copy(self->name);
  clone->c_length = self->c_length;

  if (self->nodes)
  {
    clone->nodes = list_new(self->nodes->count);
    clone->pairs = (self->pairs) ? list_new(self->pairs->count) : NULL;

    for (iN=0; iN<self->nodes->count; iN++)
    {
      n = list_index(self->nodes, iN);

      switch (n->type)
      {
        case PLAIN_NODE:
        case LINK_NODE:
          list_append(clone->nodes, node_new(LINK_NODE, n->data, n->size));
          break;
        case VAR_NODE:
          n = node_new(
                VAR_NODE,
                n->data ? str_copy((const char *) n->data) : NULL,
                n->size
              );
          p = (pair_t) list_index(self->pairs, iP++);
          list_append(clone->pairs, pair_new(p->key, strlen(p->key), n));
          list_append(clone->nodes, n);
          break;
        case BLOCK_NODE:
          n = node_new(
                BLOCK_NODE, templight_clone((templight_t) n->data), n->size
              );
          if (n->size != -1)
          {
            p = (pair_t) list_index(self->pairs, iP++);
            list_append(clone->pairs, pair_new(p->key, strlen(p->key), n));
          }
          list_append(clone->nodes, n);
          break;
      }
    }
  }
  else
  {
    clone->nodes = NULL;
    clone->pairs = NULL;
  }

  return clone;
}

/* -------------------------------------------------------------------------- */

templight_t
templight_new_block(templight_t self, const char * block)
{
  if (!self || !self->pairs) return NULL;

  int i;



  templight_t b;
  pair_t      p;

  for(i=0; i<self->pairs->count; i++)
  {
    p = (pair_t) list_index(self->pairs, i);

    if (p->node->type != BLOCK_NODE || strcmp( p->key, block)) continue;
    /* pair found */

    /* block node found */
    if (p->node->data)
    {
      p->node->size++;

      b = templight_clone( (templight_t ) p->node->data );

      /* find insert position */
      for (i=0; i<self->nodes->count; i++)
      {
        if (list_index(self->nodes, i) == p->node)
        {
          list_insert(
            self->nodes,
            node_new(BLOCK_NODE, b, -1),
            i+p->node->size
          );
          return b;
        }
      }
    }
    break;
  }

  fprintf(
    stderr,"[templight] block '%s' not found in '%s'\n", block, self->name
  );

  return NULL;
}


/* -------------------------------------------------------------------------- */

static int
_set_value(templight_t self, const char *var_name, char *value)
{

  if (!self->pairs) return 0;

  int i, r = 0, l=strlen(value);

  for(i=0; i<self->pairs->count; i++)
  {
    if (
        (((pair_t)list_index(self->pairs, i))->node->type == VAR_NODE) &&
        strcmp( ((pair_t)list_index(self->pairs, i))->key, var_name)==0 )
    {

      if (((pair_t)list_index(self->pairs, i))->node->data)
      {
        self->c_length -= strlen( 
          (const char*) ((pair_t)list_index(self->pairs, i))->node->data
        );
        free( ((pair_t)list_index(self->pairs, i))->node->data );
      }

      if (r)
        ((pair_t)list_index(self->pairs,i))->node->data=str_copy(value);
      else
        ((pair_t)list_index(self->pairs,i))->node->data=value;

      ((pair_t)list_index(self->pairs,i))->node->size = l;

      self->c_length += l;

      r++;
    }
  }

  if (r==0)
    free(value);

  return r;
}

/* -------------------------------------------------------------------------- */

int
templight_set_printf(templight_t self, const char *var_name, 
                                       const char *format, ...)
{
  va_list list;
  va_start(list, format);
  int result = templight_set_vprintf(self, var_name, format, list);
  va_end(list);
  return result;
}

/* -------------------------------------------------------------------------- */

int
templight_set_vprintf(templight_t self, const char *var_name, 
                                        const char *format,
                                        va_list     list)
{
  if (!self || !var_name || !format) return 0;

  char * value = str_vprintf(format, list);

  return (value) ? _set_value(self, var_name, value) : 0;
}

/* -------------------------------------------------------------------------- */

int
templight_set_string(templight_t self, const char *var_name, const char *value)
{
  if (!self || !var_name) return 0;

  char *v = str_copy(value ? value : "");

  return _set_value(self, var_name, v);
}

/* -------------------------------------------------------------------------- */

int
templight_set_integer(templight_t self, const char *var_name, const int value)
{
  if (!self || !var_name) return 0;
  
  char * v = str_printf("%d", value);
  return _set_value(self, var_name, v);
}

/* -------------------------------------------------------------------------- */

int
templight_set_float(templight_t self, const char *var_name, const double value)
{
  if (!self || !var_name) return 0;
  
  char * v = str_printf("%f", value);
  return _set_value(self, var_name, v);
}

/* -------------------------------------------------------------------------- */

int
templight_set_block(templight_t self, const char *var_name, templight_t value)
{
  if (!self || !var_name) return 0;

  int i, r = 0;
  pair_t   p;

  for(i=0; i<self->pairs->count; i++)
  {
    if ( strcmp( ((pair_t)list_index(self->pairs, i))->key, var_name)==0 )
    {
      p = (pair_t)list_index(self->pairs, i);

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

      p->node->data = (r) ? templight_clone(value) : value;

      r++;

    }
  }

  return r;
}

/* -------------------------------------------------------------------------- */

void
templight_to_stream(templight_t self, aisl_stream_t s)
{
  if (!self->nodes) return ;

  int    i = 0;
  node_t n;

  for ( i=0; i<self->nodes->count; i++)
  {
    if ( (n = list_index(self->nodes, i))==NULL)
      continue;

    switch (n->type)
    {
      case PLAIN_NODE:
      case LINK_NODE:
        aisl_write(s, (const char*) n->data, n->size);
        break;
      case VAR_NODE:
        if (n->data)
          aisl_write(s, (const char *) n->data, n->size);
        break;
      case BLOCK_NODE:
        if ( n->size == -1)
        {
          templight_to_stream( ((templight_t) n->data), s );
        }
        break;
    }
  }
}

/* -------------------------------------------------------------------------- */

int
templight_get_content_length(templight_t self)
{
  if (!self) return 0;
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
