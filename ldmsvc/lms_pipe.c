
#include <windows.h>

#include "../shared/shared.h"

#include "lms_heap.h"
#include "lms_pipe.h"


#define DOUTST2(m, r)           DOUTST("lms_pipe", m, r)
#define DOUTGLE2(m)             DOUTGLE("lms_pipe", m)


typedef struct {
    HANDLE      pipe;
    OVERLAPPED  ol;
    int         io_pending;
    char        name[];
} pipe_t;



void* pipe_start(const char* name){
    pipe_t  *pipe;

    if((pipe = (pipe_t*)heap_alloc(sizeof(pipe_t) + strlen(name) + 1)) == NULL){
        DOUTST("lms_pipe", "HeapAlloc", 0);
        return NULL;
    }

    memset(&pipe->ol, 0, sizeof(pipe->ol));
    if((pipe->ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL){
        DOUTGLE2("CreateEvent (overlapped)");
        heap_free(pipe);
        return 0;
    }

    pipe->io_pending = 0;

    strcpy(pipe->name, name);
    pipe->pipe = NULL;

    return pipe;
}

int pipe_stop(void* p){
    pipe_t* pipe = (pipe_t*)p;

    CloseHandle(pipe->ol.hEvent);

    if(pipe->pipe != NULL){
        if(pipe->io_pending){
            CancelIo(pipe->pipe);
        }
        CloseHandle(pipe->pipe);
    }

    heap_free(pipe);
    return 1;
}

int pipe_open(void* p){
    pipe_t* pipe = (pipe_t*)p;

    if(pipe->pipe == NULL){
        HANDLE  h;
        DWORD   dw;

        if(!WaitNamedPipe(pipe->name, 0)){
            DOUTGLE2("WaitNamedPipe");
            return 0;
        }

        h = CreateFile(pipe->name, GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if(h == INVALID_HANDLE_VALUE){
            DOUTGLE2("CreateFile");
            return 0;
        }

        dw = PIPE_READMODE_MESSAGE;
        if(!SetNamedPipeHandleState(h, &dw, NULL, NULL)){
            DOUTGLE2("SetNamedPipeHandleState");
            CloseHandle(h);
            return 0;
        }

        pipe->pipe = h;
    }

    return 1;
}

int pipe_transact(void *p, void* out_buf, int out_buf_sz, void* in_buf, int in_buf_sz, int *read){
    pipe_t*     pipe = (pipe_t*)p;
    DWORD       bytes_read;
    DWORD       gle;

    if(pipe->pipe == NULL){
        dout("ERROR (pipe_transact): the pipe has not been opened\n");
        return PIPE_ERROR;
    }

    if(!pipe->io_pending){
        if(TransactNamedPipe(pipe->pipe, out_buf, out_buf_sz, in_buf, in_buf_sz, 0, &pipe->ol)){
        }else{
            switch(gle = GetLastError()){
            case ERROR_IO_PENDING:
                pipe->io_pending = 1;
                return PIPE_PENDING;
            default:
                DOUTST2("TransactNamedPipe", gle);
                return PIPE_ERROR;
            }
        }
    }else{
        if(GetOverlappedResult(pipe->pipe, &pipe->ol, &bytes_read, FALSE)){
        }else{
            switch(gle = GetLastError()){
            case ERROR_IO_INCOMPLETE:
                return PIPE_PENDING;
            default:
                DOUTST2("TransactNamedPipe", gle);
                pipe->io_pending = 0;
                return PIPE_ERROR;
            }
        }
    }

    pipe->io_pending = 0;
    *read = bytes_read;
    return PIPE_OK;
}



