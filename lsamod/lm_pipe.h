

#ifndef __LM_PIPE_H__
#define __LM_PIPE_H__

//#include <windows.h>
#include "lm_sam.h"

#define PIPE_IO_BUFFER_SIZE      1024        // this value is recommended to be specified in ConnectNamedPipe

typedef int (*lm_pipe_onthink_t)(struct lm_pipe_s *This);
typedef int (*lm_pipe_onterminate_t)(struct lm_pipe_s *This);

typedef struct {
    lm_pipe_onthink_t       think;
    lm_pipe_onterminate_t   terminate;
} lm_pipe_state_t;

typedef struct lm_pipe_s {
    lm_pipe_state_t     *state;

    lm_sam_t            *sam_peer;


    int                 pending;        // for now, valid for conn state only
    HANDLE              pipe;
    OVERLAPPED          ol;

    char                buffer[PIPE_IO_BUFFER_SIZE];
    DWORD               buffer_size;
} lm_pipe_t;

void init_lm_pipe(lm_pipe_t* This, lm_sam_t *sam_peer);

#endif // __LM_PIPE_H__

