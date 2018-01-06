#ifndef _CSTUFF_BASE64_H_
#define _CSTUFF_BASE64_H_

/* -------------------------------------------------------------------------- */

size_t
base64_get_encoded_size( const unsigned char * in, size_t i_len );

/* -------------------------------------------------------------------------- */

int
base64_encode( const unsigned char  * in,
               int                    i_len,
               unsigned char       ** out,
               int                  * o_len );

/* -------------------------------------------------------------------------- */

int
base64_decode( const unsigned char  * in,
               int                    i_len,
               unsigned char       ** out,
               int                  * o_len );

/* -------------------------------------------------------------------------- */

#endif
