#ifndef _AISL_BUFFER_H_
#define _AISL_BUFFER_H_

#include <stdint.h>

struct _buffer_t
{
  char       *data;
  uint32_t    size;
  uint32_t    len;
};

typedef struct _buffer_t * buffer_t;

/* constructor -------------------------------------------------------------- */

buffer_t
buffer_new(uint32_t size);

/* destructor --------------------------------------------------------------- */

void
buffer_free(buffer_t self);

/* set buffer data and size ------------------------------------------------- */

void
buffer_set(buffer_t self, const char * data, uint32_t d_len);

/* move data from one buffer to another ------------------------------------- */

void
buffer_move(buffer_t target, buffer_t source);

/* put data to buffer without resizing, returns size of moved data ---------- */

uint32_t
buffer_put(buffer_t source, const char * data, uint32_t d_len);

/* add data to buffer with resizing if it is necessary ---------------------- */

void
buffer_add(buffer_t source, const char * data, uint32_t d_len);

/* -------------------------------------------------------------------------- */

#endif
