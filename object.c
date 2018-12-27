/*
 * object.c
 * Copyright (C) 2018 Ilja Kartašov <ik@lowenware.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "object.h"


DEFINE_CLASS(object)


static int
object_init(object_t self)
{
  self->_links = 0;  // to calm down compiler
  return 0;
}


static void
object_free(object_t self)
{
  free(self);
}


object_t
object_allocate(object_class_t object_class, size_t size)
{
  object_t self = calloc(1, size);

  if (self)
  {
    int rc;
    self->object_class = object_class;
    if ((rc = object_class->init(self)) != 0)
    {
      fprintf(
          stderr, 
          "L3D !- #%000X : failed to allocate `%s` object \n",
          rc,
          object_class->name
      );
      object_class->free(self);
      self = NULL;
    }
  }

  return self;
}


void *
object_link(object_t self)
{
  self->_links++;
  return (void *) self;
}


void
object_unlink(object_t self)
{
  fprintf(
      stderr
    , "unlink %s [%p] (%u)\n"
    , self->object_class->name
    , (void *) self
    , self->_links
  );

  if (!self->_links || --self->_links == 0)
    self->object_class->free( self );
}
