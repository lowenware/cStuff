#include <stdlib.h>
#include "str-utils.h"
#include "pair.h"

/* -------------------------------------------------------------------------- */

pair_t
pair_new(const char * key, const char * value)
{
  pair_t self = calloc(1, sizeof(struct pair));

  if (self)
  {
    if (key && !(self->key = str_copy(key)))
      goto except;

    if (value && !(self->value = str_copy(value)))
      goto except;
  }

  goto finally;

except:
  pair_free(self);

finally:
  return self;
}

/* -------------------------------------------------------------------------- */


void
pair_free(pair_t self)
{
  if (self->key)
    free(self->key);

  if (self->value)
    free(self->value);

  free(self);
}

/* -------------------------------------------------------------------------- */
