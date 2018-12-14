#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>

#include "str-utils.h"
#include "fs-utils.h"

#ifdef CSTUFF_FS_WITH_MAKE_FILE_PATH

int
fs_make_file_path( const char * filename )
{
  int    result = -1;
  char * path,
       * pch;
  FILE * f;

  errno = 0;
  if ( (path = str_copy( filename )) != NULL)
  {
    pch = path;

    while (pch)
    {
      if (*pch != '/')
      {
        if ( (pch = strchr(pch, '/')) != NULL )
        {
          *pch = 0;
          if (mkdir( path, 0755 ) == -1 && errno != EEXIST) break;

          *pch = '/';
        }
        else
        {
          if ( (f = fopen(path, "a")) != NULL )
          {
            fclose(f);
            result = 0;
          }
        }
      }
      else if ( *(++pch) == 0 ) break;
    }

    free(path);
  }
  return result;
}

#endif


char *
fs_get_file_content( const char * filename, size_t * p_size )
{
  size_t fsize   = 0;
  char * content = NULL;
  FILE * f = fopen(filename, "rb");

  if (f)
  {

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  //same as rewind(f);

    if ((content = malloc(fsize + 1)) != NULL)
    {
      fread(content, fsize, 1, f);
      content[fsize] = 0;
      if (p_size)
        *p_size = fsize;
    }

    fclose(f);
  }

  return content;
}


bool
fs_file_exists( const char * filename )
{
  return ( access( filename, F_OK ) != -1 ) ? true : false;
}

