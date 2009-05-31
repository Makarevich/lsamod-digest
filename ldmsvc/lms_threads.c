
#include <windows.h>

#include "../shared/shared.h"
#include "../shared/shpipe.h"

#include "lms_heap.h"
#include "lms_net.h"
#include "lms_threads.h"
#include "lms_pipe.h"

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
    void*           pipe;
    HANDLE          *ths;
    int             ths_count;
    int             max_conn;
    HANDLE          shutdown;           // manual reset event
    HANDLE          thread_ready;       // auto reset event
    HANDLE          pipe_lock;          // mutex
    void*           current_sock;
} threads_t;

void* threads_start(void* net, void* pipe, int max_conn){
    threads_t   *threads;

    if((max_conn <= 0) || (max_conn > MAX_CONN)) max_conn = MAX_CONN;

    if((threads = (threads_t*)HAlloc(sizeof(threads_t))) == NULL){
        DOUTHEAP();
        return NULL;
    }

    if((threads->ths = (HANDLE*)HAlloc(sizeof(HANDLE) * max_conn)) == NULL){
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

    if((threads->pipe_lock = CreateMutex(NULL, FALSE, NULL)) == NULL){
        DOUTGLE2("CreateEvent (thread_ready)");
        CloseHandle(threads->thread_ready);
        CloseHandle(threads->shutdown);
        HFree(threads->ths);
        HFree(threads);
        return NULL;
    }

    threads->max_conn = max_conn;
    threads->ths_count = 0;
    threads->net = net;
    threads->pipe = pipe;

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

    CloseHandle(threads->pipe_lock);
    CloseHandle(threads->thread_ready);
    CloseHandle(threads->shutdown);
    HFree(threads->ths);
    HFree(threads);

    return 1;
}



static DWORD WINAPI connection_daemon(LPVOID params){
    HANDLE      sd;
    HANDLE      pipe_lock;
    void*       sock;
    void*       pipe;


    char        buffer[NET_LINE_BUFFER_SIZE];
    int         count = 0;

    {
        threads_t*  threads = (threads_t*)params;

        sd          = threads->shutdown;
        pipe_lock   = threads->pipe_lock;
        sock        = threads->current_sock;
        pipe        = threads->pipe;

        SetEvent(threads->thread_ready);
    }

#define NET_SEND(buf, sz)     \
    if(!net_sendline(sock, buf, sz)){           \
        DOUTST_("daemon", "net_sendline");      \
        goto l_close;                           \
    }                                           \

#define NET_SEND_STR(str)     \
    {                                           \
        char buf[] = str;                       \
        NET_SEND(buf, sizeof(buf) - 1)          \
    }

    for(;;){
        switch(net_recvline(sock, buffer, &count, sizeof(buffer)/sizeof(buffer[0]))){
        case RECVLINE_ERROR:
            net_closesocket(sock);
            return 0;
        case RECVLINE_MAXCOUNT:
            {
                NET_SEND_STR("ERR incoming line is too long\n");

                count = 0;
                continue;
            }


        case RECVLINE_WOULDBLOCK:
            break;
        case RECVLINE_CLOSED:
            goto l_close;
        case RECVLINE_OK:
            {
                // char hash[] = "AAAAAAAA" "BBBBBBBB" "CCCCCCCC" "DDDDDDDD" "\n";
                // NET_SEND(hash, sizeof(hash) - 1);

                char                *name;
                pipe_in_buffer_t    in;
                pipe_out_buffer_t   out;
                int                 r, r2;
                char                outbuf[64];

                r = ansi2unicode(buffer, count, in.wchars, sizeof(in.wchars));
                if(r == 0){
                    NET_SEND_STR("ERR ansi-to-unicode name conversion failed\n");
                    count = 0;
                    continue;
                }

                in.wchar_count = r;

                while(WaitForSingleObject(pipe_lock, 0) == WAIT_TIMEOUT){
                    if(WaitForSingleObject(sd, 10) == WAIT_OBJECT_0){
                        goto l_close;
                    }
                }

retry_transaction:
                r2 = pipe_transact(pipe, &in, sizeof(in), &out, sizeof(out), &r);
                switch(r2){
                case PIPE_PENDING:
                    goto retry_transaction;
                case PIPE_OK:
                    ReleaseMutex(pipe_lock);
                    break;
                case PIPE_ERROR:
                default:
                    ReleaseMutex(pipe_lock);
                    NET_SEND_STR("ERR pipe error\n");
                    count = 0;
                    continue;
                }

                if(out.result){
                    strcpy(outbuf, va("ERR SYS %08X\n", out.result));
                }else{
                    HASHHEX     hex;

                    CvtHex(out.hash, hex);
                    strcpy(outbuf, hex);
                    strcat(outbuf, "\n");
                }

                NET_SEND(outbuf, strlen(outbuf));

                count = 0;
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

    if(net_is_ready(threads->net)){
        while(threads->ths_count < threads->max_conn){
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



