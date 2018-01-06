#include <stdio.h>
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
  int i;
  for(i = 0; i < 16; i++)
  {
    sscanf(&hash[i*2], "%2hhx", (unsigned char *) &digest[i]);
  }
}

/* -------------------------------------------------------------------------- */

void
md5_digest_to_hash(char digest[16], char * hash)
{
  int i;
  for (i=0; i<16; i++)
  {
    sprintf(&hash[i*2], "%02x", digest[i] & 0xFF);
  }
  hash[32] = 0;
}

/* -------------------------------------------------------------------------- */

