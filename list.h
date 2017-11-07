/* list.h - header file of the list_t module
 * Copyright (c) 2017 Löwenware Ltd (https://lowenware.com)
 *
 * REPOSITORY:
 *   https://github.com/lowenware.com:cStuff.git
 * MAINTAINER:
 *   Ilja Kartaschoff <ik@lowenware.com>
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
 * */

#ifndef _CSTUFF_LIST_H_
#define _CSTUFF_LIST_H_

#include <stdint.h>

/* MODULE: list_t
 * Dynamic storage for pointers */

/* structure ---------------------------------------------------------------- */

struct _list_t
{
    void     **list;  /* list itself */
    uint32_t   size;  /* number of bytes allocated for list */
    uint32_t   count; /* number of defined items in list */
};

typedef struct _list_t * list_t;

/* callback to free memory used by stored item ------------------------------ */

typedef void (* list_destructor_t)(void * list_item);

/* functions ---------------------------------------------------------------- */

/* get list item by index macro
 * @self   : list_t instance
 * @index  : item index
 * @result : stored pointer 
 * */
#define list_index(self, index) (self->list[index])

/* -------------------------------------------------------------------------- */

/* create new list_t object
 * @size : initial list size
 * @result new list_t instance
 * */
list_t
list_new(uint32_t size);

/* -------------------------------------------------------------------------- */

/* free resources allocated for list_t instance
 * @self       : list_t instance
 * @destructor : if set, destructor method will be called for each not-null
 *               list entry
 * */
void
list_free(list_t self, list_destructor_t destructor);

/* -------------------------------------------------------------------------- */

#ifdef LIST_WITH_INSERT

/* insert new item into list at some position, 
 * memory will be allocated if necessary
 * @self     : list_t instance
 * @item     : pointer to be inserted
 * @position : number from 0(prepend) to list size (append)
 * */
void
list_insert( list_t self, void * item, uint32_t position );

#endif

/* -------------------------------------------------------------------------- */

#ifdef LIST_WITH_APPEND

/* append item to queue list
 * @self   : list_t object
 * @item   : pointer to be appended
 * @result : position at which pointer was stored
 * */
uint32_t
list_append(list_t self, void * item);

#endif

/* -------------------------------------------------------------------------- */

#ifdef LIST_WITH_REMOVE

/* remove pointer from list
 * @self : list_t object
 * @item : to be removed
 * */
void
list_remove( list_t self, void * item );

#endif

/* -------------------------------------------------------------------------- */

#ifdef LIST_WITH_REMOVE_INDEX

/* remove pointer from list by index
 * @self  : list_t object
 * @index : index of pointer to be removed
 * */
void *
list_remove_index( list_t self, uint32_t index );

#endif

/* -------------------------------------------------------------------------- */

#ifdef LIST_WITH_SET_ITEM

/* set pointer by index. if element does not exist, list won't be extended
 * @self  : list_t object
 * @index : index of pointer to be removed
 * @value : value to be set
 * */
void
list_set_item(list_t self, uint32_t index, void * value);

#endif

/* -------------------------------------------------------------------------- */

#endif
