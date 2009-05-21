#include <windows.h>
//#include <ntsecapi.h>
#include "../shared/shared.h"
#include "lm_sam.h"

static volatile int     unload_flag = 0;
static HANDLE           working_thread_handle = NULL;

const char dout_file[] = "lsamod.log";

static DWORD WINAPI working_thread(LPVOID param){

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
            if(working_thread_handle != NULL){
                unload_flag = 1;
                if(WaitForSingleObject(working_thread_handle, 1000) == WAIT_TIMEOUT){
                    TerminateThread(working_thread_handle, 0);
                }
                CloseHandle(working_thread_handle);
            }

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

