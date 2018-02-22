#include "fs-utils.h"

#ifdef CSTUFF_FS_WITH_MAKE_FILE_PATH

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include "str-utils.h"

unsigned short 
file_exists(const char *path) {

    if(!access(path, F_OK | R_OK )) {
        return 1;
    }

    return 0;

}

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
