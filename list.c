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
list_new(int   size)
{
  list_t self = calloc(1, sizeof(struct list));
  if (self)
  {
    if (size)
    {
      self->size = size;

      if ( (self->list = calloc(size, sizeof(void *))) == NULL )
      {
       free(self);
       self = NULL;
      }
    }
  }
  return self;
}

/* -------------------------------------------------------------------------- */

void
list_free(list_t self, list_destructor_t destructor)
{
  int i;
  if (self)
  {
    if (self->list)
    {
      printf("list_free(%d)\n", self->count);
      if (destructor)
      {
        for(i=0; i<self->count; i++)
        {
          if (self->list[i])
            destructor(self->list[i]);
        }
      }
      free(self->list);
    }
    free(self);
  }
}

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LIST_WITH_INSERT

int
list_insert( list_t self, void * item, int position )
{
  int r;

  if ( (r = list_append(self, item)) == -1)
    return -1;

  if (position >= r || position < 0)
    return r;

  memmove(
    &self->list[position+1],
    &self->list[position],
    (self->count-position-1)*sizeof(void*)
  );

  self->list[position] = item;

  return position;
}


#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_LIST_WITH_APPEND

int
list_append(list_t self, void * item)
{
  void ** new_list;
  int     new_size = self->count+1;


  if (new_size > self->size)
  {
    if ( (new_list = realloc( self->list, new_size*sizeof(void*) )) == NULL )
      return -1;


    self->list = new_list;
    self->size = new_size;
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
  int i;

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
list_remove_index( list_t self, int index )
{
  void * result;
  int i;

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
list_set_item(list_t self, int index, void * value)
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
