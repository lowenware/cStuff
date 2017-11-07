#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "list.h"
#include "str_utils.h"


#define SHIFT 0            /* index of indent with line shift */
#define EQUAL 1            /* index of indent before '=' sign */
#define VALUE 2            /* index of indent before value  */
#define COMNT 3            /* index of indent before comment  */

/* constants ---------------------------------------------------------------- */

const int  cMaxLength   = 255;
const char cConfigTrue[]  = "1";
const char cConfigFalse[] = "0";

/* types -------------------------------------------------------------------- */

struct _config_t
{
  list_t   lines;         /* list of lines */
  uint32_t root;          /* index of line with current section */
  uint32_t max_id;        /* node id generator */
};

typedef struct
{
  uint8_t  indent[4];       /* array of line indents */
  char    *key;           /* key name */
  char    *value;         /* value name */
  char    *comment;         /* comment name */
  uint32_t node_id;         /* raw number of section */
  bool     do_not_unref;      /* keep in memory */

} config_line_t;

typedef struct
{
  const char  * key;
  config_line_t * root;

} config_find_t;


/* static ------------------------------------------------------------------- */


/* main parsing method, called by config_read
 * @param self config_t object
 * @param line buffer with null-terminated line
 * @param l pointer to config_line_t structure, that will be created and parsed
 * @param stack object of list_t that will be used as a stack for config nodes
 * @result CONFIG_SUCCESS or sconfig_tSyntaxError
 * */
static config_status_t
config_read_line( config_t self, char       * line,
                 config_line_t ** l,
                 list_t       stack )
{
  /* printf ("<<< %s", line); */
  int       i = strlen(line)-1; /* pointer to current indent counter */
  char      * p = line;       /* line iterator */
  char      * b = NULL;       /* pointer to begin of word */
  char      save;         /* variable to save character */
  config_line_t * node = NULL;


  *l = calloc(1, sizeof(config_line_t));
  list_append(self->lines, *l);

  if (line[ i ] != '\n') return CONFIG_SYNTAX_ERROR;
  line[ i ] = 0;
  i = 0;

  /* indent */
  while ((*p == ' ' || *p == '\t') && *(p++)) (*l)->indent[SHIFT]++;



  if (*p==0) return CONFIG_SUCCESS;

  /* key */
  b = p;
  p += strcspn(b, ";:= ");

  /* if first symbol after indent is a stop symbol(could not be space) */
  if (b == p)
  {
    /* we have line with command */
    if (*p == ';')
    {
      (*l)->comment = str_copy(b);
      return CONFIG_SUCCESS;
    } 
    else
      return CONFIG_SYNTAX_ERROR;
  }

  /* find root node */

  while (1)
  {
    if (!stack->count) break;

    node = list_index(stack, stack->count-1);

    if (node->indent[SHIFT] >= (*l)->indent[SHIFT])
    {
      list_remove_index(stack, stack->count-1);
      node = NULL;
    }
    else
      break;
  }

  /* we have boundaries of key, so save it */
  save = *p;
  *p = 0;

  if (node)
  {
    (*l)->key = str_printf("%s.%s", node->key, b);
    (*l)->node_id = node->node_id;
  }
  else
    (*l)->key = str_copy(b);

  *p = save;

  (*l)->indent[EQUAL] += strcspn(p, ":=");
  p += (*l)->indent[EQUAL];

  /* check if line is over */
  if (*p == 0) return CONFIG_SYNTAX_ERROR;

  save = *p;

  p++;

  while ((*p == ' ' || *p == '\t') && *(p++)) (*l)->indent[VALUE]++;

  b = p;

  p = strchr(b, ';');

  /* save value */
  if (save == '=')
  {
    if (p)
    {
      i = 0;
      save = *p;
      *p = 0;
    }
    (*l)->value = str_set((*l)->value, b);
    (*l)->indent[COMNT] = str_chop((*l)->value);
    if (p) *p = save;
  }
  else
  {
    if(!p) return CONFIG_SUCCESS;
    else if (p != b) return CONFIG_SYNTAX_ERROR;
  }

  if (p && *p == ';')
  {
    (*l)->comment = str_copy(p);
  }



  return CONFIG_SUCCESS;
}

/* -------------------------------------------------------------------------- */

/* get current root node. If node is not select, number of config lines will
 * be written into line
 * @param self config_t object
 * @param line pointer to line counter
 * @return pointer to line structure of root node or NULL if not found
 * */
static config_line_t *
config_get_root(config_t self, uint32_t * line )
{
  uint32_t i;
  config_line_t  * root=NULL;

  *line = 0;

  for (i=0; i<self->lines->count; i++)
  {
    root = list_index(self->lines, i);

    if (root->node_id == self->root && root->key)
      break;
    else
      root=NULL;
  }

  *line = (root) ? i : 0;

  return root;
}

/* -------------------------------------------------------------------------- */

static config_line_t *
_get_line_by_key(config_t self, const char * key, config_line_t ** o_root)
{
  uint32_t     i  = 0,
           shift;

  config_line_t * l = NULL,
         * root = config_get_root(self, &i);

  /* output root */
  if (o_root)
    *o_root = root;

  /*printf("\nLookup key: '%s', root=%s, size=%d\n", key, root ? root->key : "null", self->lines->count); */

  for (i=(root)?i+1:0; i<self->lines->count; i++)
  {
    l = list_index( self->lines, i);

    if (!l || !l->key)
    {
      l = NULL;
      continue;
    }

    shift = 0;

    if (root)
    {
      shift = strlen(root->key)+1;

       /*printf(" Indents: root=%d line=%d\n", root->indent[0] & 0xFF, l->indent[0] & 0xFF);  */
      if (root->indent[0] >= l->indent[0]) return NULL;
       /*printf(" root indent is OK\n"); */
    }
    
     /*printf("  compare with '%s'\n", l->key); */

    if ((shift <= strlen(l->key)) && strcmp(&l->key[shift], key)==0)
      break;
    else
      l=NULL;
  }

  /*printf("Return: %p\n", l); */
  return l;
}

/* -------------------------------------------------------------------------- */

/* find proper place to insert new node or key-value pair.
 * proper place is the end of node, or after last occurence of key in node
 * @param self config_t object
 * @param key  key text, NULL if key is not presented in config already
 * @param line pointer to line counter
 * @return indent
 * */
uint8_t
config_get_insert_position( config_t self, const char * key, uint32_t * line )
{
  uint8_t indent = 0;
  config_line_t  * l, * root=NULL;

  if (key)
  {
    l = _get_line_by_key(self, key, &root);
    
    /* if(l) printf("<< same key @ %d\n", *line); */

    if (!l) key = NULL;
  }
  else
    root = config_get_root(self, line);
  
  (*line)++;

  if (root)
  {
    /*printf("<< root @ %d:%d\n", root->node_id, *line);*/
    indent = root->indent[0]+4;
  }

  if (!key && !root)
  {
    *line = self->lines->count;
    return 0;
  }

  /* need last occurance of this key */

  uint32_t i, shift=0;

  for (i=*line; i < self->lines->count; i++)
  {
    l = list_index(self->lines, i);

    if (root)
    {
      /* end of parent node - right place */
      if (l->indent[0] <= root->indent[0])
        break;

      shift = strlen(root->key)+1;
    }
    
    if (key)
    {
      /*printf("compare %s and %s \n", &key[shift], find->key); */
      if (strncmp (&l->key[shift], key, strlen(key)))
        break;
    }
  }

  if (!l) (*line)++;

  return indent;
}

/* -------------------------------------------------------------------------- */

/* free resources used by line structure
 * @param line pointer to config_line_t structure
 * */
static void
_free_line( config_line_t * line )
{
  if (line)
  {
    if (line->key)   free(line->key);
    if (!line->do_not_unref)
      if (line->value)   free(line->value);
    if (line->comment) free(line->comment);
    free(line);
    *((void **)&line) = NULL;
  }

}


/* implementation ----------------------------------------------------------- */


config_t 
config_new(const char * file)
{
  config_t self = calloc(1, sizeof(struct _config_t));

  self->lines = list_new(32);

  return self;
}

/* -------------------------------------------------------------------------- */

void
config_free(config_t self)
{
  if (self)
  {
    list_free(self->lines, (list_destructor_t)_free_line);
    free(self);
    *((void**) &self) = NULL;
  }
}

/* -------------------------------------------------------------------------- */

uint32_t
config_get_lines(config_t self)
{
  return self->lines->count;
}


/* read methods ------------------------------------------------------------- */

config_status_t
config_read(config_t self, const char * file)
{
  config_status_t   s = CONFIG_SUCCESS;
  FILE            * fd;
  char              line[ cMaxLength + 1 ];
  list_t            stack = list_new(3);

  config_line_t  * l;

  fd = fopen(file, "r");
  if (!fd) return CONFIG_FILE_ERROR;


  while(fgets(line, cMaxLength, fd))
  {
    /* printf("read config line %s\n", line); */
    s = config_read_line(self, line, &l, stack);

    if (l->key && !l->value) /* new node */
    {
      l->node_id = ++self->max_id;
      list_append(stack, l);
    }

    /*
    if (l)
      printf(
        "%d:\t%d\t%d\t%d\t%d\t%s = %s;\tc=%s\n",
        l->node_id,
        l->indent[0],
        l->indent[1],
        l->indent[2],
        l->indent[3],
        (l->key) ? l->key : "NULL",
        (l->value) ? l->value : "NULL",
        (l->comment) ? l->comment : "NULL"
      );
    */


    if (s != CONFIG_SUCCESS)
      break;
  }
  /*  printf("read finish\n"); */

  fclose(fd);

  list_free(stack, NULL);
  self->root = 0;

  return s;
}

/* -------------------------------------------------------------------------- */

char *
config_get_string(config_t self, const char * key)
{
  config_line_t * l = _get_line_by_key(self, key, NULL);

  if (l && l->value)
  {
    l->do_not_unref = true;
    return l->value;
  }
  else
    return NULL;
}

/* -------------------------------------------------------------------------- */

const char *
config_get_chars(config_t self, const char * key, const char * fb)
{
  config_line_t * l = _get_line_by_key(self, key, NULL);

  return (l && l->value) ? (const char *) l->value : fb;
}
/* ToDO: not free resources */

/* -------------------------------------------------------------------------- */

int
config_get_integer(config_t self, const char * key, int fb)
{
  int  result = 0;
  const char * value = config_get_chars(self, key, "");

  result = strtol(value, NULL, 10);
  if (!result && strcmp(value, "0")) result = fb;

  return result;
}

/* -------------------------------------------------------------------------- */

bool
config_get_boolean(config_t self, const char * key, bool fb)
{
  const char * value = config_get_chars(
      self, key, (fb) ? cConfigTrue : cConfigFalse
  );

  return (strcmp(value, cConfigTrue)==0) ? true : false;
}

/* -------------------------------------------------------------------------- */

bool
config_set_root(config_t self, const char * root)
{
  if (!root)
  {
    self->root=0;
    return true;
  }

  if (*root == '.')
    root++;
  else
    self->root=0;
  
  config_line_t  * l = _get_line_by_key(self, root, NULL);

  if (l)
  {
    self->root = l->node_id;
    return true;
  }
  else
    return false;
  
}


/* write methods ------------------------------------------------------------ */

config_status_t
config_write(config_t self, const char * file)
{
  FILE      * fd;
  char      line[ cMaxLength + 1 ];
  char      * p;
  char      * k;
  int       i;
  config_line_t * l;

  fd = fopen(file, "w");
  if (!fd) return CONFIG_FILE_ERROR;

  for (i=0; i<self->lines->count; i++)
  {
    p = line;

    l = list_index(self->lines, i);

    memset(p, ' ', l->indent[0]);
    p += l->indent[0];

    if (l->key)
    {
      k = strrchr(l->key, '.');
      k = (!k) ? l->key : k+1;
      strcpy(p, k);
      p += strlen(k);

      if (!l->value)
      {
        *(p++) = ':';
      }
      else
      {
        memset(p, ' ', l->indent[1]);
        p += l->indent[1];
        *(p++) = '=';
      }
      memset(p, ' ', l->indent[2]);
      p += l->indent[2];

      if (l->value)
      {
        if (l->value)
          strcpy(p, l->value);
        p += (l->value) ? strlen(l->value) : 0;
      }
    }

    if (l->comment)
    {
      memset(p, ' ', l->indent[3]);
      p += l->indent[3];
      if (l->comment)
        strcpy(p, l->comment);
      p += (l->comment) ? strlen(l->comment) : 0;
    }
    *(p++) = '\n';
    *(p)   = '\0';

    fputs(line, fd);
  }

  fclose(fd);

  return CONFIG_SUCCESS;
}

/* -------------------------------------------------------------------------- */

void
config_set_string(config_t self, const char * key, const char * value)
{
  config_line_t * root,
         * l = _get_line_by_key(self, key, &root);

  if (!l)
  {
    uint32_t       i=0;

    config_get_insert_position(self, NULL, &i);

    l = calloc(1, sizeof(config_line_t));
    l->value = str_set(l->value, value);

    if (root)
    {
      l->key = str_printf("%s.%s", root->key, key);
      /*
      l->key = string_new_with_size(
         string_get(root->key),
         string_get_length(root->key)+strlen(key)+1
      );
      string_append(l->key, ".");
      string_append(l->key, key);
      */
      l->indent[0] = root->indent[0]+4;
      l->node_id   = root->node_id;
    }
    else
      l->key = str_set(l->key, key);

    l->indent[1]=1;
    l->indent[2]=1;
    list_insert(self->lines, l, i);
    //printf("<<< %s=%s @ %u:%u:%u\n", key, value, i, l->node_id, l->indent[0] & 0xFF);
  }
  else
    l->value = str_set(l->value, value);

}

/* -------------------------------------------------------------------------- */

void
config_set_integer(config_t self, const char * key, int value)
{
  char buffer[10+1];

  sprintf(buffer, "%d", value);

  config_set_string(self, key, buffer);
}

/* -------------------------------------------------------------------------- */

void
config_set_boolean(config_t self, const char * key, bool value)
{
  config_set_string(self, key, (value)?cConfigTrue:cConfigFalse);
}

/* -------------------------------------------------------------------------- */

/* node name has to be full name
 * new node will be set as current
 * */

void
config_add_node(config_t self, const char * name)
{
  config_line_t  * l = calloc(1, sizeof(config_line_t));
  uint32_t       i=0;

  l->indent[0] = config_get_insert_position(self, name, &i);

  l->key = str_copy(name);
  l->node_id = ++self->max_id;
  l->indent[1] = 1;
  l->indent[2] = l->indent[1];

  list_insert(self->lines, l, i);

  self->root = l->node_id;
}


/* -------------------------------------------------------------------------- */
