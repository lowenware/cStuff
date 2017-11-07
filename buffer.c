#include <stdlib.h>
#include <string.h>

#include "buffer.h"

/* constructor -------------------------------------------------------------- */

buffer_t
buffer_new(uint32_t size)
{
  buffer_t self = malloc(sizeof(struct _buffer_t));

  self->size = size;
  self->len  = 0;
  self->data = size ? malloc(sizeof(char)*size) : NULL;

  return self;
}

/* destructor --------------------------------------------------------------- */

void
buffer_free(buffer_t self)
{
  if (self->data)
    free(self->data);

  free(self);
}

/* set buffer data and size ------------------------------------------------- */

void
buffer_set(buffer_t self, const char * data, uint32_t d_len)
{
  if ( d_len )
  {
    if ( self->data )
    {
      if ( self->size < d_len )
      {
        free(self->data);
        self->data = NULL;
      }
    }

    if ( self->data == NULL )
    {
      self->size = ((uint32_t)( d_len / 1024 )) * 1024;
      if (d_len % 1024)
        self->size += 1024;

      self->data = malloc( self->size * sizeof(char) );
    }
    if(data)
      memcpy(self->data, data, d_len);
  }
    
  self->len = d_len;
}

/* move data from one buffer to another ------------------------------------- */

void
buffer_move(buffer_t target, buffer_t source)
{
  uint32_t l = target->size-target->len;

  if (source->len < l) l = source->len;

  memcpy(&target->data[target->len], source->data, l);
  target->len+=l;

  if (l < source->len)
  {
    memcpy(source->data, &source->data[l], source->len-l);
  }

  source->len -= l;
}

/* put data to buffer without resizing, returns size of moved data ---------- */

uint32_t
buffer_put(buffer_t self, const char * data, uint32_t d_len)
{
  uint32_t l = self->size - self->len;

  if (l)
  {
    if (d_len < l) l = d_len;

    memcpy(&self->data[self->len], data, l);
    self->len += d_len;
  }

  return l;
}

/* add data to buffer with resizing if it is necessary ---------------------- */

void
buffer_add(buffer_t self, const char * data, uint32_t d_len)
{
  uint32_t l = self->size - self->len;

  if (l < d_len)
  {
    l = d_len - l;
    self->size += ((uint32_t)( l / 1024 )) * 1024;
    if (l % 1024)
      self->size += 1024;

    char * ptr = malloc(self->size * sizeof(char));


    if(self->data)
    {
      if (self->len)
        memcpy(ptr, self->data, self->len);
      free(self->data);
    }

    self->data = ptr;
  }

  memcpy(&self->data[self->len], data, d_len);
  self->len += d_len;
}

/* -------------------------------------------------------------------------- */
