
//
// ut2.c
//
//   Perfomrs a pipe transaction.
//
//   The program sends pipe_in_buffer_t and receives pipe_out_buffer_t
//   in a single transaction, which emulates the service program.
//   Everything is hardcoded.
//


#include <windows.h>
#include "../shared/shared.h"
#include "../shared/shpipe.h"

#define DOUTGLE2(m)     DOUTGLE("ut2" ,m)

int main(){
    HANDLE              pipe;
    pipe_in_buffer_t    in;
    pipe_out_buffer_t   out;
    DWORD               cnt;
    HASHHEX             hex;

    pipe = CreateFile(SHARED_PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(pipe == INVALID_HANDLE_VALUE){
        DOUTGLE2("CreateFile(pipe)");
        return -1;
    }

    cnt = PIPE_READMODE_MESSAGE;
    if(!SetNamedPipeHandleState(pipe, &cnt, NULL, NULL)){
        DOUTGLE2("SetNamedPipeHandleState");
        CloseHandle(pipe);
        return -1;
    }

    memcpy(in.wchars, L"user2", 10);
    in.wchar_count = 5;

    if(!TransactNamedPipe(pipe, &in, sizeof(in), &out, sizeof(out), &cnt, NULL)){
        DOUTGLE2("TransactNamedPipe");
        CloseHandle(pipe);
        return -1;
    }

    CvtHex(out.hash, hex);

    dout(va("Pipe returned: %08X\n"
            "Hash: %s\n",
            out.result,
            hex));

    CloseHandle(pipe);

    return 0;
}

