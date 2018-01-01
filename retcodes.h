#ifndef _CSTUFF_RETCODE_H_
#define _CSTUFF_RETCODE_H_


#define CSTUFF_PARSE_ERROR  -11
#define CSTUFF_NOT_FOUND    -10
#define CSTUFF_EXTCALL_ERROR -4
#define CSTUFF_SYSCALL_ERROR -3
#define CSTUFF_MALLOC_ERROR  -2
#define CSTUFF_NULL_OBJECT   -1
#define CSTUFF_SUCCESS        0

#define RAISE(x, label) { result = x; goto label; }

#endif
