
#ifndef __LMS_NET_H__
#define __LMS_NET_H__

#define RECVLINE_ERROR          0
#define RECVLINE_MAXCOUNT       1
#define RECVLINE_WOULDBLOCK     2
#define RECVLINE_CLOSED         3
#define RECVLINE_OK             4

void* net_start();
int net_stop(void* h);
int net_close(void* h);
int net_bind(void* h, const char* addr, int port);
int net_is_ready(void* h);
int net_accept(void *h, void** psocket);
int net_recvline(void* s, char* buffer, int* pcount, int max_count);
int net_sendline(void* s, const char* buffer, int count);
int net_closesocket(void* s);


#endif // __LMS_NET_H__

