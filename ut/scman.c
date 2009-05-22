
//
// scman.c
//
//   This program adds or removes a service from the system.
//   Everything is hardcoded; modify SERVICE_* and recompile.
//


#include <windows.h>
#include "../shared/shared.h"


#define SERVICE_NAME                "ldmsvc"
#define SERVICE_DISPLAY_NAME        "lsamod-digest service"
#define SERVICE_BINARY_PATH         "c:\\windows\\system32\\ldmsvc.exe"


#define APICALL_NULL(res, api)  \
    dout(#api ": ");                                        \
    if((res = api) == 0){                                   \
        dout(va("failed with %u\n", GetLastError()));

#define APIEND()        \
    }else{              \
        dout("OK\n");   \
    }

void no_op(){
    dout("Don't know what to do.\n");
}

int create_svc(){
    SC_HANDLE       scm;
    SC_HANDLE       svc;

    APICALL_NULL(scm, OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE));
        return -1;
    APIEND();

    APICALL_NULL(svc, CreateService(scm, SERVICE_NAME, SERVICE_DISPLAY_NAME, 0, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, SERVICE_BINARY_PATH, NULL, NULL, NULL, NULL, NULL));
        CloseServiceHandle(scm);
        return -1;
    APIEND();

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return 0;
}

int delete_svc(){
    SC_HANDLE       scm;
    SC_HANDLE       svc;
    BOOL            b;

    APICALL_NULL(scm, OpenSCManager(NULL, NULL, 0));
        return -1;
    APIEND();

    APICALL_NULL(svc, OpenService(scm, SERVICE_NAME, DELETE));
        CloseServiceHandle(scm);
        return -1;
    APIEND();

    APICALL_NULL(b, DeleteService(svc));
        CloseServiceHandle(svc);
        CloseServiceHandle(scm);
        return -1;
    APIEND();

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);

    return 0;
}

int main(){
    char        *cmd;
    char        c;

    cmd = GetCommandLine();

    while(1){
        c = *(cmd++);
        if((c == '-') || (c == '/')){
            break;
        }else if(c == 0){
            no_op();
            return -1;
        }
    }

    switch(c = *cmd){
    case 'C':
        dout("Creating service...\n");
        return create_svc();
    case 'D':
        dout("Deleting service...\n");
        return delete_svc();
    default:
        dout(va("Unknown op: %c\n", c));
        no_op();
        return -1;
    }
}

