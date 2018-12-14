#ifndef _CSTUFF_FS_UTILS_H_
#define _CSTUFF_FS_UTILS_H_

#include <stdbool.h>

#ifdef CSTUFF_FS_WITH_MAKE_FILE_PATH

int
fs_make_file_path( const char * filename );

#endif


char *
fs_get_file_content( const char * filename, size_t * p_size );


bool
fs_file_exists( const char * filename );

#endif
