#include <windows.h>
#include "../shared/shared.h"

int main(){
    char*           buf;
    HANDLE          mutex;
    shmem_ldr_t     shmem;

    shmem.op = LDROP_NOP;

    buf = GetCommandLine();

    for(buf = GetCommandLine(); (*buf != '/') && (*buf != '-'); buf++){
        if(*buf == 0){
            dout("You must specify an operation.\n"
                 "Specify \"/?\" parameter for additional help.\n");
            return 1;
        }
    }
    buf++;

    switch(*buf){
    case '?':
        dout("This program issues an operation to the lsamod.\n"
             "\n"
             "Possible operations are:\n"
             "L - call LoadLibrary;\n"
             "F - call FreeLibrary;\n"
             "C - call an export funcation (via GetProcAddress).\n"
             "\n"
             "Ussage example:\n"
             "    nudge.exe /L C:\\mylib.dll\n"
             "    nudge.exe /C my_proc\n"
             "    nudge.exe /F\n"
             "\n"
             );
        return 0;
    case 'L':
        do{
            buf++;
        }while((*buf == ' ') || (*buf == '\t'));

        strncpy(shmem.data, buf, sizeof(shmem.data));
        shmem.op = LDROP_LOADLIB;
        break;
    case 'C':
        do{
            buf++;
        }while((*buf == ' ') || (*buf == '\t'));

        strncpy(shmem.data, buf, sizeof(shmem.data));
        shmem.op = LDROP_CALLPROC;
        break;
    case 'F':
        shmem.op = LDROP_FREELIB;
        break;
    default:
        dout("Unknown operation was specified.\n"
             "You may use only [L]oad, [F]ree or [C]all.\n");
        return 1;
    }

    if(!shmem_open(&buf, &mutex)){
        return -1;
    }

    if(WaitForSingleObject(mutex, 2000) == WAIT_OBJECT_0){
        memcpy(buf, &shmem, sizeof(shmem));
        ReleaseMutex(mutex);
        // dout("The operation has successfully been issued.\n");
    }else{
        DOUTGLE("nudge", "WaitForSingleObject");
    }

    shmem_close(buf, mutex);

    return 0;
}

