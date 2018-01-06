#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include "retcodes.h"
#include "base64.h"

/* -------------------------------------------------------------------------- */

size_t
base64_get_encoded_size( const unsigned char * in, size_t i_len )
{
  size_t b64_padding;

  if ( in[i_len - 1 ] == '=' )
  {
    b64_padding = ( in[ i_len - 2] == '=' ) ? 2 : 1;
  }
  else
    b64_padding = 0;

  return i_len * 3 / 4 - b64_padding;
}

/* -------------------------------------------------------------------------- */

int
base64_encode( const unsigned char  * in,
               int                    i_len,
               unsigned char       ** out,
               int                  * o_len )
{
  BIO     *bio,
          *b64;
  BUF_MEM *bptr;

  int      result = CSTUFF_EXTCALL_ERROR;

  if ( !(b64 = BIO_new(BIO_f_base64())) )
    goto finally;

  if ( !(bio = BIO_new(BIO_s_mem())) )
    goto release_b64;

  if ( !(b64 = BIO_push(b64, bio)) )
    goto release_bio;

  BIO_set_flags(b64,BIO_FLAGS_BASE64_NO_NL);

  if ( !(BIO_write(b64, in, i_len) > 0) )
    goto release_b64;

  if ( !(BIO_flush(b64) == 1) )
    goto release_b64;

  BIO_get_mem_ptr(b64, &bptr);

  if (o_len)
    *o_len = bptr->length;

  if ( !(*out = malloc(bptr->length+1)) )
  {
    result = CSTUFF_MALLOC_ERROR;
    goto release_b64;
  }

  memcpy(*out, bptr->data, bptr->length);
  (*out)[ bptr->length ] = 0;

  result = CSTUFF_SUCCESS;

  goto release_b64;

release_bio:
  BIO_free_all(bio);
release_b64:
  BIO_free_all(b64);
finally:
  return result;
}

/* -------------------------------------------------------------------------- */

int
base64_decode( const unsigned char  * in,
               int                    i_len,
               unsigned char       ** out,
               int                  * o_len )
{
  int    result = CSTUFF_SUCCESS;

  BIO  * b64,
       * bio;

  size_t length = base64_get_encoded_size(in, i_len);

  if ( !(bio = BIO_new_mem_buf((void*)in, i_len))  )
  {
    result = CSTUFF_EXTCALL_ERROR;
    goto finally;
  }

  if ( !(*out = malloc( length * sizeof( unsigned char ) )) )
  {
    result = CSTUFF_MALLOC_ERROR;
    goto release;
  }

  if ( (b64 = BIO_new(BIO_f_base64())) != NULL )
  {
    if ( (bio = BIO_push(b64, bio)) != NULL )
    {
      BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
      if ( length == BIO_read(bio, *out, length) )
      {
        if (o_len) *o_len = length;

        goto release;
      }
    }
    else
      BIO_free(b64);
  }


/* e_extcall: */

  result = CSTUFF_EXTCALL_ERROR;
  free( *out );
  *out = NULL;

release:
  BIO_free_all(bio);

finally:
  return result;
}


/* -------------------------------------------------------------------------- */
