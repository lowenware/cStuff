#ifndef _CSTUFF_FS_UTILS_H_
#define _CSTUFF_FS_UTILS_H_

unsigned short
file_exists(const char *path);

#ifdef CSTUFF_FS_WITH_MAKE_FILE_PATH

int
fs_make_file_path( const char * filename );

#endif

#endif
