
#ifndef __LMS_THREADS_H__
#define __LMS_THREADS_H__

void* threads_start(void* net, void* pipe, int max_conn);
int threads_stop(void *th);
int threads_think(void* th);

#endif // __LMS_THREADS_H__


