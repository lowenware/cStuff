/*
 * object.h
 * Copyright (C) 2018 Ilja Kartašov <ik@lowenware.com>
 *
 */

#ifndef OBJECT_H_B477DE05_FC3C_42CD_A812_BBCCF314D4DD
#define OBJECT_H_B477DE05_FC3C_42CD_A812_BBCCF314D4DD

#define CSTUFF_GET_MACRO(_1, _2, NAME, ...) NAME
#define CSTUFF_STRINGIFYX( X ) #X
#define CSTUFF_STRINGIFY(X) CSTUFF_STRINGIFYX(X)

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

// common macroses
#define DECLARE_TYPES( CLASS_NAME ) \
  extern  struct CLASS_NAME##_class g_##CLASS_NAME##_class; \
  typedef struct CLASS_NAME##_class * CLASS_NAME##_class_t; \
  typedef struct CLASS_NAME * CLASS_NAME##_t; \


#define DECLARE_FROM( CLASS_NAME, PARENT_CLASS ) \
  struct CLASS_NAME ## _class \
  { \
    struct PARENT_CLASS##_class parent; \
  }; \
  DECLARE_TYPES( CLASS_NAME )


#define DECLARE_CLASS(...) \
  CSTUFF_GET_MACRO( \
    __VA_ARGS__, \
    DECLARE_FROM, \
    DECLARE_TYPES, \
    PLACEHOLDER \
  )(__VA_ARGS__)


#define DEFINE_CLASS_TOP(CLASS_NAME) \
  static int \
  CLASS_NAME ##_init(CLASS_NAME##_t); \
  \
  static void \
  CLASS_NAME ##_free(CLASS_NAME##_t); \
  \
  struct CLASS_NAME##_class \
    g_##CLASS_NAME##_class = CLASS_NAME##_class_init(CLASS_NAME); \


#define DEFINE_CLASS_FROM(CLASS_NAME, PARENT_CLASS) \
  static int \
  CLASS_NAME##_init(CLASS_NAME##_t); \
  \
  static void \
  CLASS_NAME##_free(CLASS_NAME##_t); \
  \
  struct CLASS_NAME##_class g_##CLASS_NAME##_class = { \
    .parent = PARENT_CLASS##_class_init(CLASS_NAME) \
  }; \


#define DEFINE_CLASS(...) \
  CSTUFF_GET_MACRO( \
    __VA_ARGS__, \
    DEFINE_CLASS_FROM, \
    DEFINE_CLASS_TOP, \
    PLACEHOLDER \
  )(__VA_ARGS__)
// object macroses


#define cast( OBJECT, CLASS_NAME ) ((CLASS_NAME##_t)(OBJECT))


#define SUPER_OBJECT_CLASS( OBJECT, CLASS_NAME ) (\
    (CLASS_NAME##_class_t) OBJECT(OBJECT)->object_class \
) \

#define SUPER_CLASS( CLASS_NAME ) ( (object_class_t) &g_##CLASS_NAME##_class )


#define super(...) \
  CSTUFF_GET_MACRO( \
    __VA_ARGS__, \
    SUPER_OBJECT_CLASS, \
    SUPER_CLASS, \
    PLACEHOLDER \
  )(__VA_ARGS__)


// ----------------------------------------------------------------------------

#define object_class_init(CLASS_NAME) \
{ \
  .name = CSTUFF_STRINGIFY(CLASS_NAME), \
  .init = (object_init_t) CLASS_NAME##_init, \
  .free = (object_free_t) CLASS_NAME##_free \
} \


#define OBJECT( X ) cast( X, object )

DECLARE_CLASS(object)

typedef int
(*object_init_t)(object_t self);

typedef void
(*object_free_t)(object_t self);

// begin class declaration

struct object_class
{
  const char     * name;
  object_init_t    init;
  object_free_t    free;
};

// end class declaration

struct object
{
  /* public */
  object_class_t    object_class;

  /* private */
  uint32_t          _links;
};




#define object_new( CLASS_NAME ) \
  (CLASS_NAME##_t) object_allocate( \
    (object_class_t) &g_##CLASS_NAME##_class, sizeof(struct CLASS_NAME) \
  ) \


object_t
object_allocate(object_class_t object_class, size_t size);


void *
object_link(object_t self);


void
object_unlink(object_t self);


#endif /* !OBJECT_H */
