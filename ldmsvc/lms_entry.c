#define _WIN32_WINNT 0x0400
#include <windows.h>

#include "../shared/shared.h"
#include "../shared/shpipe.h"

#ifdef UNICODE
#error This application should not be compile as a unicode application
#endif

#define LDMSVC_SERVICE_NAME      "ldmsvc"



const char dout_file[] = "ldmsvc.log";


static SERVICE_STATUS_HANDLE h_service = NULL;
static volatile DWORD svc_state = SERVICE_START_PENDING;

/* set service status */
static BOOL set_state2(DWORD state, DWORD exit_code){
    SERVICE_STATUS ss;
    DWORD accept;

    switch(state){
    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:
        accept = 0;
        break;
    default:
        accept = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    }

    memset(&ss, 0, sizeof(ss));
    ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    ss.dwCurrentState = state;
    ss.dwControlsAccepted = accept;
    ss.dwWin32ExitCode = exit_code;

    svc_state = ss.dwCurrentState;

    return SetServiceStatus(h_service, &ss);
}
static BOOL set_state1(DWORD state){
    return set_state2(state, 0);
}
static BOOL set_state(){
    return set_state1(svc_state);
}

/* service control handler */
static VOID WINAPI handler(DWORD control){
    switch(control){
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        set_state1(SERVICE_STOP_PENDING);
        break;
    default:
        set_state();
    }
}

/* service_main */
static VOID WINAPI service_main(DWORD argc, LPTSTR *argv){
    h_service = RegisterServiceCtrlHandler(LDMSVC_SERVICE_NAME, handler);
    if(h_service == NULL){
        return;
    }

    set_state1(SERVICE_START_PENDING);        // SERVICE_START_PENDING

    /*
    // initialize here
    if(!config_parse_args(argc, argv)){
        set_state2(SERVICE_STOPPED, 1);
        return;
    }

    // initialize the network
    if(!net_initialize()){
        set_state2(SERVICE_STOPPED, 1);
        return;
    }
    */

    set_state1(SERVICE_RUNNING);              // SERVICE_RUNNING

    while(svc_state == SERVICE_RUNNING){
        Sleep(10);
    }

    set_state1(SERVICE_STOP_PENDING);         // SERVICE_STOP_PENDING

    // close all here
    //net_finalize();

    set_state2(SERVICE_STOPPED, 0);           // SERVICE_STOPPED
}

/* print_usage */
static void print_usage(){
    static char message[] =
        "Linux digest service\n"
        "\n"
        "This executable must be run as a Windows service \n"
        "  (service name: \"" LDMSVC_SERVICE_NAME "\" without quotes)\n"
        "\n"
        "The following paramerets are accepted:\n"
        "  -p <num>             tcp port to listen (default: 3111)\n"
        "  -pipe <pipe-name>    pipe name that is used to connect\n"
        "                        to lsamod.dll (default: " SHARED_PIPE_NAME ")\n"
        "  -mc <num>            maximum connection that\n"
        "                        service can accept (default: 8)\n"
        ;
    DWORD dw;

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), message, sizeof(message) - 1, &dw, 0);
}

/* entry point */
int main(){
    SERVICE_TABLE_ENTRY service_table[] = {
        {LDMSVC_SERVICE_NAME, service_main},
        {NULL, NULL}
    };

    if(!StartServiceCtrlDispatcher(service_table)){
        if(GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT){
            print_usage();
        }
    }

    return 0;
}

