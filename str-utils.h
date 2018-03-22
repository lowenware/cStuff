/* str_utils.h - header file of the C string module
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

#ifndef _CSTUFF_STR_UTILS_H_
#define _CSTUFF_STR_UTILS_H_

#include <stdint.h>
#include <stdarg.h>

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_SET

#ifndef CSTUFF_STR_UTILS_WITH_COPY
#define CSTUFF_STR_UTILS_WITH_COPY
#endif

/* set init string to src. 
 * */

/* if init is not set or smaller than src, it will be reallocated
 * */
char *
str_set(char * init, const char * src);

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_COPY

/* copy src string to newly allocated one
 * */
char *
str_copy(const char * src);

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_NCOPY

/* copy num bytes from src string to newly allocated one 
 * */
char *
str_ncopy(const char * src, uint32_t num);

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_PRINTF

char *
str_printf(const char * format,  ...);

char *
str_vprintf(const char * format,  va_list vl);

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_CAT

#ifndef CSTUFF_STR_UTILS_WITH_NCAT
#define CSTUFF_STR_UTILS_WITH_NCAT
#endif


/* for source will be called u_free automatically */
char *
str_cat(char * source, const char * target);

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_NCAT

/* for source will be called u_free automatically */
char *
str_ncat(char * source, const char * target, uint32_t length);

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_CMPI

/* compare strings ignoring case */
int
str_cmpi(const char * source, const char * target);

# endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_TO_INT

int
str_to_int(const char * ptr, int l, uint32_t * result);

#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_TO_INT64

int
str_to_int64(const char * ptr, int l, uint64_t * result);

#endif

/* -------------------------------------------------------------------------- */

#ifdef CSTUFF_STR_UTILS_WITH_CHOP

/* chop spaces from string, return number of prepending spaces */
uint32_t
str_chop(char * source);

# endif

/* -------------------------------------------------------------------------- */


#endif
