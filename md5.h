#ifndef _CSTUFF_MD5_H_
#define _CSTUFF_MD5_H_

/* -------------------------------------------------------------------------- */

void
md5_from_string(const char * source, int source_length, char digest[16]);

/* -------------------------------------------------------------------------- */

void
md5_hash_to_digest(const char * hash, char digest[16]);

/* -------------------------------------------------------------------------- */

void
md5_digest_to_hash(char * digest, char * hash);

/* -------------------------------------------------------------------------- */

#endif
