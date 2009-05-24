
#include <windows.h>

#include "../shared/shared.h"

#include "lms_heap.h"
#include "lms_net.h"
#include "lms_threads.h"

#define HAlloc(sz)      heap_alloc(sz)
#define HFree(p)        heap_free(p)

#define DOUTHEAP()      DOUTST("lms_threads", "HeapAlloc", 0)
#define DOUTST_(p,m)    dout("ERROR (" p "): " m " failed\n")
#define DOUTGLE2(m)     DOUTGLE("lms_threads", m)


#define THREAD_INIT_TIMEOUT             1000
#define THREAD_SHUTDOWN_TIMEOUT         3000

#define NET_LINE_BUFFER_SIZE            512
#define MAX_CONN                        32




typedef struct {
    void*           net;
    HANDLE          *ths;
    int             ths_count;
    int             max_conn;
    HANDLE          shutdown;           // manual reset
    HANDLE          thread_ready;       // auto reset
    void*           current_sock;
} threads_t;

void* threads_start(void* net, int max_conn){
    threads_t   *threads;
    int         r;

    if((threads = (threads_t*)HAlloc(sizeof(threads_t))) == NULL){
        DOUTHEAP();
        return NULL;
    }

    if((threads->ths = (HANDLE*)HAlloc(r = sizeof(HANDLE) * max_conn)) == NULL){
        DOUTHEAP();
        HFree(threads);
        return NULL;
    }

    if((threads->shutdown = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL){
        DOUTGLE2("CreateEvent (shutdown)");
        HFree(threads->ths);
        HFree(threads);
        return NULL;
    }

    if((threads->thread_ready = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL){
        DOUTGLE2("CreateEvent (thread_ready)");
        CloseHandle(threads->shutdown);
        HFree(threads->ths);
        HFree(threads);
        return NULL;
    }

    threads->max_conn = max_conn;
    threads->ths_count = 0;
    threads->net = net;

    return threads;
}

int threads_stop(void *th){
    threads_t*  threads = (threads_t*)th;
    DWORD       r;
    int         i;

    if(SetEvent(threads->shutdown) == 0){
        DOUTGLE2("SetEvent");
    }

    if(threads->ths_count > 0){
        r = WaitForMultipleObjects(threads->ths_count, threads->ths, TRUE, THREAD_SHUTDOWN_TIMEOUT);
        if((r >= WAIT_OBJECT_0) && (r < (WAIT_OBJECT_0 + threads->ths_count))){
            for(i = 0; i < threads->ths_count; i++){
                CloseHandle(threads->ths[i]);
            }
        }else{
            for(i = 0; i < threads->ths_count; i++){
                TerminateThread(threads->ths[i], 0);
                CloseHandle(threads->ths[i]);
            }
        }
    }

    CloseHandle(threads->thread_ready);
    CloseHandle(threads->shutdown);
    HFree(threads->ths);
    HFree(threads);

    return 1;
}



static DWORD WINAPI connection_daemon(LPVOID params){
    HANDLE      sd;
    void*       sock;

    char        buffer[NET_LINE_BUFFER_SIZE];
    int         count = 0;

    {
        threads_t*  threads = (threads_t*)params;

        sd = threads->shutdown;
        sock = threads->current_sock;

        SetEvent(threads->thread_ready);
    }

#define NET_SEND(buf, size)     \
    if(!net_sendline(sock, buf, size)){         \
        DOUTST_("daemon", "net_sendline");      \
        goto l_close;                           \
    }

    for(;;){
        switch(net_recvline(sock, buffer, &count, sizeof(buffer)/sizeof(buffer[0]))){
        case RECVLINE_ERROR:
            net_closesocket(sock);
            return 0;
        case RECVLINE_MAXCOUNT:
            {
                const char line[] = "ERR incoming line is too long\n";

                NET_SEND(line, strlen(line));

                count = 0;
                continue;
            }


        case RECVLINE_WOULDBLOCK:
            break;
        case RECVLINE_CLOSED:
            goto l_close;
        case RECVLINE_OK:
            {
                const char hash[] = "AAAAAAAA" "BBBBBBBB" "CCCCCCCC" "DDDDDDDD" "\n";

                NET_SEND(hash, 33);
                continue;
            }
        default:
            dout("ERROR (daemon): net_recvline returned unexpected result\n");
            goto l_close;
        }

        if(WaitForSingleObject(sd, 10) == WAIT_OBJECT_0){
            goto l_close;
        }
    }

#undef NET_SEND

l_close:
    net_closesocket(sock);
    return 0;
}

int threads_think(void* th){
    threads_t*  threads = (threads_t*)th;

    // NOTE: the network is expected to be ready

    while(threads->ths_count > 0){
        DWORD r;

        r = WaitForMultipleObjects(threads->ths_count, threads->ths, FALSE, 0);
        if(r == WAIT_TIMEOUT){
            break;
        }else if((r >= WAIT_OBJECT_0) && (r < (WAIT_OBJECT_0 + threads->ths_count))){
            r -= WAIT_OBJECT_0;

            CloseHandle(threads->ths[r]);
            if((threads->ths_count - r) > 1){
                threads->ths[r] = threads->ths[threads->ths_count - 1];
            }
            threads->ths_count--;
        }else{
            DOUTGLE2("WaitForMultipleObjects");
            return 0;
        }
    }

    if((threads->ths_count < threads->max_conn) && (net_is_ready(threads->net))){
        for(;;){
            DWORD       tid;
            HANDLE      h;
            void*       sock;

            if(!net_accept(threads->net, &sock)){
                return 0;
            }

            if(sock == NULL){
                break;
            }

            threads->current_sock = sock;

            h = CreateThread(NULL, 0, connection_daemon, threads, 0, &tid);
            if(h == NULL){
                DOUTGLE2("CreateThread");
                return 0;
            }

            if(WaitForSingleObject(threads->thread_ready, THREAD_INIT_TIMEOUT) != WAIT_OBJECT_0){
                dout("WARNING: A created thread did not respond in a timely fashion\n");
                TerminateThread(h, 0);
                CloseHandle(h);
                net_closesocket(sock);
                continue;
            }

            threads->ths[threads->ths_count++] = h;
        }
    }

    return 1;
}



