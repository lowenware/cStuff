/* - templight.h --------------------------------------------------------------
 *
 * Copyright (c) 2017 Löwenware Ltd (https://lowenware.com)
 *
 * REPOSITORY:
 *   https://github.com/lowenware.com:cStuff.git
 * MAINTAINER:
 *   Elias Löwe <elias@lowenware.com>
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
 * -------------------------------------------------------------------------- */

#ifndef _CSTUFF_TEMPLIGHT_H_
#define _CSTUFF_TEMPLIGHT_H_

#include <stdarg.h>
#include <aisl/aisl.h>

#include "retcodes.h"

/* types -------------------------------------------------------------------- */

typedef struct templight * templight_t;

/* functions ---------------------------------------------------------------- */

int
templight_new(templight_t * self, const char * name, const char * root);

/* -------------------------------------------------------------------------- */

void
templight_free(templight_t self);

/* -------------------------------------------------------------------------- */

void
templight_set_greedy( templight_t self, int greedy );

/* -------------------------------------------------------------------------- */

int
templight_get_greedy( templight_t self );

/* -------------------------------------------------------------------------- */

int
templight_get_content_length(templight_t self);

/* -------------------------------------------------------------------------- */

const char *
templight_get_name(templight_t self);

/* -------------------------------------------------------------------------- */

int
templight_strip( templight_t self );

/* -------------------------------------------------------------------------- */

int
templight_clone(templight_t self, templight_t * clone);

/* -------------------------------------------------------------------------- */

int
templight_new_block(templight_t self, templight_t * block, const char * label);

/* -------------------------------------------------------------------------- */

int
templight_set_string(templight_t self, const char *var_name, 
                                       const char *value);

/* -------------------------------------------------------------------------- */


int
templight_set_integer(templight_t self, const char *var_name,
                                        const int   value);

/* -------------------------------------------------------------------------- */

int
templight_set_block(templight_t self, const char  *var_name,
                                      templight_t  value);

/* -------------------------------------------------------------------------- */

int
templight_set_printf( templight_t self, const char *var_name, 
                                        const char *format, ...);

/* -------------------------------------------------------------------------- */

int
templight_set_vprintf( templight_t self, const char *var_name, 
                                         const char *format,
                                         va_list     list);

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_TEMPLIGHT_WITH_TO_AISL_STREAM

int
templight_to_aisl_stream(templight_t self, aisl_stream_t s);

#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_TEMPLIGHT_WITH_TO_FSTREAM

int
templight_to_fstream(templight_t self, FILE * fstream);

#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_TEMPLIGHT_WITH_DUMP

int
templight_dump(templight_t self);

#endif

/* -------------------------------------------------------------------------- */
#endif
