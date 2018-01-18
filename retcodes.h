#ifndef _CSTUFF_RETCODE_H_
#define _CSTUFF_RETCODE_H_


#define CSTUFF_PARSE_ERROR           -105
#define CSTUFF_NOT_FOUND             -104
#define CSTUFF_EXTCALL_ERROR         -103
#define CSTUFF_SYSCALL_ERROR         -102
#define CSTUFF_MALLOC_ERROR          -101
#define CSTUFF_NULL_OBJECT           -100
#define CSTUFF_SUCCESS               0
#define CSTUFF_SUCCESS_WITH_REMARK   1
#define CSTUFF_PENDING               2

#define RAISE(x, label) { result = x; goto label; }

typedef int cstuff_retcode_t;

#endif
