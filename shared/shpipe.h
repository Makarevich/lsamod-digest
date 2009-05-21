

#ifndef __SH_PIPE_H__
#define __SH_PIPE_H__

#include <windows.h>
#include "utils.h"

#define SHARED_PIPE_NAME                "\\\\.\\pipe\\lsamod_digest_pipe"

// #define SHARED_PIPE_OUTBUF_SIZE         1024
// #define SHARED_PIPE_INBUF_SIZE          1024
#define SHARED_PIPE_DEFAULT_TO          1000


//
// NOTE: Each of the following structures must fit within PIPE_IO_BUFFER_SIZE byte range.
//       See ./lsamod/lm_pipe.h
//


typedef struct {
    DWORD       wchar_count;
    WCHAR       wchars[256];
} pipe_in_buffer_t;

typedef struct {
    DWORD       result;
    HASH        hash;
} pipe_out_buffer_t;




#endif // __SH_PIPE_H__


