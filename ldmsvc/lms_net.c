

//#include <windows.h>
#include <winsock2.h>

#include "../shared/shared.h"
#include "lms_net.h"
#include "lms_heap.h"


#define DOUTWGLE(mod,m)             DOUTST(mod, m, WSAGetLastError())
#define DOUTWGLE2(m)                DOUTWGLE("lms_net", m)

enum net_stage_e {
    STAGE_ALLOCED,      // just after the allocation
    STAGE_STARTED,      // WSAStartup called
    STAGE_SOCKETED,     // socket() succeeded
    STAGE_BOUND,        // bind() succeeded
    STAGE_READY         // listen() succeeded
};

typedef struct {
    enum net_stage_e    stage;
    SOCKET              sock;
} net_t;

//////////////////////////////////

void* net_start(){
    net_t       *net;

    net = (net_t*)heap_alloc(sizeof(net_t));
    if(net == NULL){
        DOUTST("lms_net", "HeapAlloc", 0);
        return NULL;
    }

    net->stage = STAGE_ALLOCED;
    return net;
}

int net_stop(void* h){
    net_t       *net = (net_t*)h;

    switch(net->stage){     // there are only few breaks in the switch
    case STAGE_READY:
    case STAGE_BOUND:
    case STAGE_SOCKETED:
        closesocket(net->sock);
    case STAGE_STARTED:
        WSACleanup();
    case STAGE_ALLOCED:
        heap_free(net);
        return 1;
    default:
        dout(va("ERROR (net_close): unknown network stage: %u\n", net->stage));
        return 0;
    }
}

//////////////////////////////////

int net_close(void* h){
    net_t       *net = (net_t*)h;

    if(net->stage < STAGE_SOCKETED){
        dout("ERROR (net_close): no socket has been opened yet\n");
        return 0;
    }

    closesocket(net->sock);
    net->stage = STAGE_STARTED;
    return 1;
}

//////////////////////////////////

int net_bind(void* h, const char* addr, int port){
    net_t       *net = (net_t*)h;

    switch(net->stage){     // there are only few breaks in the switch
    case STAGE_ALLOCED:
        {
            WSADATA     wsad;

            if(WSAStartup(MAKEWORD(1,1), &wsad)){
                DOUTST("lms_net", "WSAStartup", 0);
                return 0;
            }

            dout(va(
                "WSAStartup succeeded: %s\n",
                wsad.szDescription
               ));


            net->stage = STAGE_STARTED;
        }
    case STAGE_STARTED:
        {
            net->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(net->sock == INVALID_SOCKET){
                DOUTWGLE2("socket");
                return 0;
            }

            net->stage = STAGE_SOCKETED;
        }
    case STAGE_SOCKETED:
        {
            struct sockaddr_in sa;

            memset(&sa, 0, sizeof(sa));
            sa.sin_family = AF_INET;
            sa.sin_addr.s_addr = (addr != NULL) ? (inet_addr(addr)) : (INADDR_ANY);
            sa.sin_port = htons(port);

            if(sa.sin_addr.s_addr == INADDR_NONE){
                dout(va("ERROR (net_bind): inet_addr failed: \"%s\" is not a proper address\n", addr));
                return 0;
            }

            if(bind(net->sock, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR){
                DOUTWGLE2("bind");
                return 0;
            }

            net->stage = STAGE_BOUND;
        }
    case STAGE_BOUND:
        {
            if(listen(net->sock, SOMAXCONN) == SOCKET_ERROR){
                DOUTWGLE2("listen");
                return 0;
            }

            net->stage = STAGE_READY;
        }
    case STAGE_READY:
        {
            return 1;
        }
    default:
        dout(va("ERROR (net_bind): unknown network stage: %u\n", net->stage));
        return 0;
    }
}

//////////////////////////////////

int net_is_ready(void* h){
    net_t       *net = (net_t*)h;

    return net->stage == STAGE_READY;
}

//////////////////////////////////

int net_accept(void *h, void** psocket){
    net_t       *net = (net_t*)h;
    int         selected;

    if(net->stage < STAGE_READY){
        dout("ERROR (net_accept): network is not ready\n");
        return 0;
    }

    {
        TIMEVAL     now = {0, 0};
        fd_set      fd;

        FD_ZERO(&fd);
        FD_SET(net->sock, &fd);

        // FIXME: should we change the stage if select() fails?
        selected = select(0, &fd, NULL, NULL, &now);
        if(selected == SOCKET_ERROR){
            DOUTWGLE2("select");
            return 0;
        }
    }

    if(selected){
        // FIXME: should we change the stage if accept() fails?
        SOCKET peer = accept(net->sock, NULL, NULL);
        if(peer == INVALID_SOCKET){
            DOUTWGLE2("accept");
            return 0;
        }

        *psocket = (void*)peer;
    }else{
        *psocket = (void*)0;
    }

    return 1;
}

//////////////////////////////////

int net_recvline(void* s, char* buffer, int* pcount, int max_count){
    SOCKET      sock = (SOCKET)s;

    for(;;){
        char    c;
        int     r;

        if(*pcount >= max_count){
            return RECVLINE_MAXCOUNT;
        }

        {
            TIMEVAL     now = {0, 0};
            fd_set      fd;

            FD_ZERO(&fd);
            FD_SET(sock, &fd);

            // FIXME: should we change the stage if select() fails?
            switch(select(0, &fd, NULL, NULL, &now)){
            case SOCKET_ERROR:
                DOUTWGLE2("select");
                return RECVLINE_ERROR;
            case 0:
                return RECVLINE_WOULDBLOCK;
            }
        }

        switch(r = recv(sock, &c, sizeof(c), 0)){
        case SOCKET_ERROR:
            DOUTWGLE2("recv");
            return RECVLINE_ERROR;
        case 0:
            return RECVLINE_CLOSED;
        case 1:
            if(c == '\n'){
                buffer[*pcount] = '\0';
                return RECVLINE_OK;
            }

            if(c == '\r') break;    // ignore '\r' character

            buffer[(*pcount)++] = c;
            break;
        default:
            dout(va("ERROR: recv retruned unexpected value: %u\n", r));
            return RECVLINE_ERROR;
        }
    }
}

//////////////////////////////////

int net_sendline(void* s, const char* buffer, int count){
    SOCKET      sock = (SOCKET)s;

    while(count > 0){
        int     r;
        r = send(sock, buffer, count, 0);
        if(r == SOCKET_ERROR){
            DOUTWGLE2("send");
            return 0;
        }
        buffer += r;
        count -= r;
    }

    return 1;
}

//////////////////////////////////

int net_closesocket(void* s){
    SOCKET      sock = (SOCKET)s;

    if(closesocket(sock) == SOCKET_ERROR){
        DOUTWGLE2("closesocket");
        return 0;
    }

    return 1;
}



