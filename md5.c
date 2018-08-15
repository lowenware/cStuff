#include <stdio.h>
#include <ctype.h>
#include <openssl/md5.h>
#include "md5.h"

/* -------------------------------------------------------------------------- */

void
md5_from_string(const char * source, int source_length, char digest[16])
{
  MD5_CTX md5ctx;
  MD5_Init(&md5ctx);
  MD5_Update(&md5ctx, source, source_length);
  MD5_Final((unsigned char*)digest, &md5ctx);
}

/* -------------------------------------------------------------------------- */

void
md5_hash_to_digest(const char * hash, char digest[16])
{
  int    total = 16;
  char   in, out;

  while(total--)
  {
    in = *(hash++);
    out = (in < 0x3A) ? in - 0x30 : toupper(in) - 0x37;
    out = out << 4;

    in = *(hash++);
    out += (((in < 0x3A) ? in - 0x30 : toupper(in) - 0x37) & 0xFF);
    *(digest++) = out;
  }


}

/* -------------------------------------------------------------------------- */

void
md5_digest_to_hash(char * digest, char * hash)
{
  int i;
  for (i=0; i<16; i++)
  {
    sprintf(&hash[i*2], "%02x", digest[i] & 0xFF);
  }
  hash[32] = 0;
}

/* -------------------------------------------------------------------------- */
