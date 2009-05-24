#define _WIN32_WINNT 0x0400
#include <windows.h>

#include "../shared/shared.h"
#include "../shared/shpipe.h"

#include "lms_heap.h"
#include "lms_config.h"
#include "lms_net.h"
#include "lms_threads.h"
#include "lms_pipe.h"

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
    config_t        conf;
    void*           net;
    void*           threads;
    void*           pipe;

    h_service = RegisterServiceCtrlHandler(LDMSVC_SERVICE_NAME, handler);
    if(h_service == NULL){
        return;
    }

    set_state1(SERVICE_START_PENDING);        // SERVICE_START_PENDING

    // open the heap
    if(!heap_start()){
        dout("Failed to create the heap\n");
        set_state2(SERVICE_STOPPED, 1);
        return;
    }

    // parse configuration
    if(!config_parse_args(&conf, argc, argv)){
        heap_stop();
        set_state2(SERVICE_STOPPED, 1);
        return;
    }

    if(0){
        dout(va("PORT: %u\n", conf.port));
        dout(va("PIPE: %s\n", conf.pipe));
        dout(va("MAXC: %u\n", conf.maxconn));
    }

    // open network
    if((net = net_start()) == NULL){
        heap_stop();
        set_state2(SERVICE_STOPPED, 1);
        return;
    }

    // open the pipe
    if((pipe = pipe_start(conf.pipe)) == NULL){
        net_stop(net);
        heap_stop();
        set_state2(SERVICE_STOPPED, 1);
        return;
    }

    // connect the pipe
    if(!pipe_open(pipe)){
        pipe_stop(pipe);
        net_stop(net);
        heap_stop();
        set_state2(SERVICE_STOPPED, 1);
        return;
    }

    // start threads
    if((threads = threads_start(net, pipe, conf.maxconn)) == NULL){
        pipe_stop(pipe);
        net_stop(net);
        heap_stop();
        set_state2(SERVICE_STOPPED, 1);
        return;
    }

    set_state1(SERVICE_RUNNING);              // SERVICE_RUNNING

    while(svc_state == SERVICE_RUNNING){
        if(!net_is_ready(net)){
            net_bind(net, NULL, conf.port);
        }

        if(!threads_think(threads)){
            break;
        }

        Sleep(1);
    }

    set_state1(SERVICE_STOP_PENDING);         // SERVICE_STOP_PENDING

    // close everything here
    threads_stop(threads);
    pipe_stop(pipe);
    net_stop(net);
    heap_stop();

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
        "                        to lsamod.dll (default: " CONF_DEFAULT_PIPE ")\n"
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

