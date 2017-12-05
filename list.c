/* list.c - code file of the List module
 * Copyright (c) 2017 Löwenware Ltd (https://lowenware.com)
 *
 * REPOSITORY:
 *   git://lowenware.com:standard.git
 * MAINTAINER:
 *   Ilja Kartaschoff <ik@lowenware.com>
 *
 * LICENSE and DISCLAIMER:
 *   All code stored in standard.git repository is designed to solve
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
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/* -------------------------------------------------------------------------- */

list_t
list_new(uint32_t   size)
{
  list_t self = calloc(1, sizeof(struct _list_t));
  if (size)
  {
    self->list = calloc(size, sizeof(void *));
    self->size = size;
  }
  return self;
}

/* -------------------------------------------------------------------------- */

void
list_free(list_t self, list_destructor_t destructor)
{
  uint32_t i;
  if (self)
  {
    if (self->list)
    {
      for(i=0; i<self->count; i++)
      {
        if (destructor && self->list[i])
          destructor(self->list[i]);
      }
      free(self->list);
    }
    free(self);
  }
}

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LIST_WITH_INSERT

void
list_insert( list_t self, void * item, uint32_t     position )
{
  void     **list;

  if (position > self->count) position = self->count;

  if (self->count+1 > self->size)
  {
    self->size++;
    list = self->list;
    self->list = calloc(self->size, sizeof(void *));

    if (list)
    {
      if (position > 0)
      {
        memcpy(self->list, list, position*sizeof(void *));
      }

      if (position < self->count)
      {
        memcpy(
          &self->list[position+1],
          &list[position],
          (self->count-position)*sizeof(void*)
        );
      }

      free(list);
    }
  }
  else
    list = self->list;

  self->count++;

  self->list[position]=item;

}


#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LIST_WITH_APPEND

uint32_t
list_append(list_t self, void * item)
{
  if (self->count+1 > self->size)
  {
    void * * list = self->list;

    self->list = calloc(++self->size, sizeof(void *));

    if (list)
    {
      if (self->count)
        memcpy(self->list, list, self->count*sizeof(void *));
      free(list);
    }
  }

  self->list[self->count]=item;

  return self->count++;
}

#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LIST_WITH_REMOVE

void
list_remove( list_t self, void * item )
{
  uint32_t i;

  for (i=0; i<self->count; i++)
  {
    if (self->list[i]==item)
    {
      list_remove_index(self, i);
      break;
    }
  }
}


#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LIST_WITH_REMOVE_INDEX

void *
list_remove_index( list_t self, uint32_t index )
{
  void * result;
  uint32_t i;

  if (index < self->count)
  {
    result = self->list[index];
    self->count--;
    for (i=index; i<self->count; i++)
    {
      self->list[i]=self->list[i+1];
    }
  }
  else
    result = NULL;

  return result;
}



#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LIST_WITH_SET_ITEM

void
list_set_item(list_t self, uint32_t index, void * value)
{
  if (self->size > index)
  {
    self->list[index] = value;
    if (index >= self->count)
      self->count = index+1;

  }
}

#endif

/* -------------------------------------------------------------------------- */
