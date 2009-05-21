
#include "lm_pipe.h"
#include "../shared/shared.h"
#include "../shared/shpipe.h"

#define DOUTST2(api, status)        DOUTST("lm_pipe", api, status)
#define DOUTGLE2(api)               DOUTGLE("lm_pipe", api)

#define DEFINE_STATE(state)     \
    static int onthink_pipe_ ## state(lm_pipe_t *This);         \
    static int onterminate_pipe_ ## state(lm_pipe_t *This);     \
    lm_pipe_state_t     state_pipe_ ## state = {                \
        onthink_pipe_ ## state,                                 \
        onterminate_pipe_ ## state                              \
    }

DEFINE_STATE(closed);
DEFINE_STATE(conn);
DEFINE_STATE(reading);
DEFINE_STATE(writing);

#undef DEFINE_STATE

void init_lm_pipe(lm_pipe_t* This, lm_sam_t *sam_peer){
    This->sam_peer = sam_peer;

    This->pending = 0;

    This->pipe = NULL;
    memset(&This->ol, 0, sizeof(This->ol));

    This->state = &state_pipe_closed;
}


static int enter_pipe_conn(lm_pipe_t *This);
static int enter_pipe_reading(lm_pipe_t *This);
static int enter_pipe_writing(lm_pipe_t *This);

//
// state 1: pipe hasn't been created
//   onthink:       try to create the pipe and transfer to the pipe_conn state;
//   onterminate:   do nothing
//

static int enter_pipe_closed(lm_pipe_t *This){
    // dout("PIPE: Entering closed state\n");

    if(This->pipe){
        CloseHandle(This->pipe);
        This->pipe = NULL;
    }

    if(This->ol.hEvent != NULL){
        CloseHandle(This->ol.hEvent);
        This->ol.hEvent = NULL;
    }

    This->state = &state_pipe_closed;
    return 1;
}

static int onthink_pipe_closed(lm_pipe_t *This){
    HANDLE      p;

    This->ol.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if(This->ol.hEvent == NULL){
        DOUTGLE2("CreateEvent");
        return 0;
    }

    p = CreateNamedPipe(SHARED_PIPE_NAME,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 
            1,
            // SHARED_PIPE_OUTBUF_SIZE,
            // SHARED_PIPE_INBUF_SIZE,
            PIPE_IO_BUFFER_SIZE,
            PIPE_IO_BUFFER_SIZE,
            SHARED_PIPE_DEFAULT_TO,
            NULL);
    if(p == INVALID_HANDLE_VALUE){
        DOUTGLE2("CreateNamedPipe");
        return 0;
    }

    This->pipe = p;

    return enter_pipe_conn(This);
}

static int onterminate_pipe_closed(lm_pipe_t *This){
    return 1;
}

//
// state 2: pipe is connecting
//   onthink:       wait for incoming connection
//   onterminate:   close the pipe and delegate to pipe_closed state
//

static int enter_pipe_conn(lm_pipe_t *This){
    // dout("PIPE: Entering conn state\n");

    This->pending = 0;
    This->state = &state_pipe_conn;
    return 1;
}

static int onthink_pipe_conn(lm_pipe_t *This){
    int         connected = 0;
    DWORD       gle;

    if(This->pending == 0){
        if(ConnectNamedPipe(This->pipe, &This->ol)){
            connected = 1;
        }else{
            switch(gle = GetLastError()){
            case ERROR_PIPE_CONNECTED:
                connected = 1;
                break;
            case ERROR_IO_PENDING:
                connected = 0;
                This->pending = 1;
                break;
            default:
                DOUTST2("ConnectNamedPipe", gle);       // FIXME: force entering closed state here
                return 0;
            }
        }
    }else{
        DWORD num;

        if(GetOverlappedResult(This->pipe, &This->ol, &num, FALSE)){
            connected = 1;
        }else{
            switch(gle = GetLastError()){
            case ERROR_PIPE_CONNECTED:
                connected = 1;
                break;
            case ERROR_IO_INCOMPLETE:
                connected = 0;
                break;
            default:
                DOUTST2("GetOverlappedResult (ConnectNamedPipe)", gle);   // FIXME: force entering closed state here
                return 0;
            }
        }
    }

    if(connected){
        This->pending = 0;
        return enter_pipe_reading(This);
    }

    return 1;
}

static int onterminate_pipe_conn(lm_pipe_t *This){
    if(This->pending){
        CancelIo(This->pipe);
        This->pending = 0;
    }

    enter_pipe_closed(This);
    return This->state->terminate(This);
}

//
// state 3: pipe is reading
//   onthink:       wait for the completion of ReadFile
//   onterminate:   cancel the IO, delegate to the pipe_conn state
//

static int pipe_reading_parse_input_buffer(lm_pipe_t *This){
    pipe_in_buffer_t    *in = (pipe_in_buffer_t*)This->buffer;
    pipe_out_buffer_t   *out = (pipe_out_buffer_t*)This->buffer;
    UNICODE_STRING      uname;
    HASH                hash;
    NTSTATUS            res;

    if(This->buffer_size != sizeof(pipe_in_buffer_t)){
        dout(va("ERROR: input buffer size mismatch: %u != %u", This->buffer_size, sizeof(pipe_in_buffer_t)));

        out->result = STATUS_INVALID_BUFFER_SIZE;
        memset(out->hash, 0, sizeof(out->hash));
    }else{
        uname.Length = in->wchar_count * sizeof(in->wchars[0]);
        uname.MaximumLength = uname.Length;
        uname.Buffer = in->wchars;

        dout("Calling sam peer...\n");
        This->sam_peer->state->data(This->sam_peer, &uname, hash, &res);   // call sam peer

        out->result = res;
        if(res == STATUS_SUCCESS){
            memcpy(out->hash, hash, sizeof(out->hash));
        }else{
            memset(out->hash, 0, sizeof(out->hash));
        }
    }

    This->buffer_size = sizeof(pipe_out_buffer_t);
    return enter_pipe_writing(This);
}

static int pipe_reading_long_input(lm_pipe_t *This){
    pipe_out_buffer_t   *out = (pipe_out_buffer_t*)This->buffer;

    while(ReadFile(This->pipe, This->buffer, sizeof(This->buffer), &This->buffer_size, NULL)){
        if(GetLastError() != ERROR_MORE_DATA) break;
    }

    out->result = STATUS_INVALID_BUFFER_SIZE;
    memset(out->hash, 0, sizeof(out->hash));
    This->buffer_size = sizeof(pipe_out_buffer_t);
    return enter_pipe_writing(This);
}

static int enter_pipe_reading(lm_pipe_t *This){
    DWORD       gle;

    // dout("PIPE: Entering reading state\n");

    if(ReadFile(This->pipe, This->buffer, sizeof(This->buffer), &This->buffer_size, &This->ol)){
        return pipe_reading_parse_input_buffer(This);
    }else{
        switch(gle = GetLastError()){
        case ERROR_MORE_DATA:
            return pipe_reading_long_input(This);
        case ERROR_BROKEN_PIPE:
            DisconnectNamedPipe(This->pipe);
            return enter_pipe_conn(This);
        case ERROR_IO_PENDING:
            This->state = &state_pipe_reading;
            return 1;
        default:
            DOUTST2("ReadFile", gle);
            return 0;
        }
    }
}

static int onthink_pipe_reading(lm_pipe_t *This){
    DWORD       gle;

    if(GetOverlappedResult(This->pipe, &This->ol, &This->buffer_size, FALSE)){
        return pipe_reading_parse_input_buffer(This);
    }else{
        switch(gle = GetLastError()){
        case ERROR_MORE_DATA:
            return pipe_reading_long_input(This);
        case ERROR_BROKEN_PIPE:
            DisconnectNamedPipe(This->pipe);
            return enter_pipe_conn(This);
        case ERROR_IO_INCOMPLETE:
            return 1;
        default:
            DOUTST2("GetOverlappedResult (ReadFile)", gle);
            return 0;
        }
    }
}

static int onterminate_pipe_reading(lm_pipe_t *This){
    CancelIo(This->pipe);

    enter_pipe_conn(This);
    return This->state->terminate(This);
}

//
// state 4: pipe is writing
//   onthink:       wait for the completion of WriteFile
//   onterminate:   cancel the IO, delegate to the pipe_conn state
//

static int enter_pipe_writing(lm_pipe_t *This){
    DWORD       gle;
    DWORD       num;

    // dout("PIPE: Entering writing state\n");

    if(WriteFile(This->pipe, This->buffer, This->buffer_size, &num, &This->ol)){
        return enter_pipe_reading(This);
    }else{
        switch(gle = GetLastError()){
        case ERROR_BROKEN_PIPE:
            DisconnectNamedPipe(This->pipe);
            return enter_pipe_conn(This);
        case ERROR_IO_PENDING:
            This->state = &state_pipe_writing;
            return 1;
        default:
            DOUTST2("WriteFile", gle);
            return 0;
        }
    }
}

static int onthink_pipe_writing(lm_pipe_t *This){
    DWORD       gle;
    DWORD       num;

    if(GetOverlappedResult(This->pipe, &This->ol, &num, FALSE)){
        return enter_pipe_reading(This);
    }else{
        switch(gle = GetLastError()){
        case ERROR_BROKEN_PIPE:
            DisconnectNamedPipe(This->pipe);
            return enter_pipe_conn(This);
        case ERROR_IO_INCOMPLETE:
            return 1;
        default:
            DOUTST2("GetOverlappedResult (WriteFile)", gle);
            return 0;
        }
    }
}

static int onterminate_pipe_writing(lm_pipe_t *This){
    CancelIo(This->pipe);

    enter_pipe_conn(This);
    return This->state->terminate(This);
}
