

#ifndef __LMS_PIPE_H__
#define __LMS_PIPE_H__

#define PIPE_ERROR      0
#define PIPE_OK         1
#define PIPE_PENDING    2

void* pipe_start(const char* name);
int pipe_stop(void* p);
int pipe_open(void* p);
int pipe_transact(void *p, void* out_buf, int out_buf_sz, void* in_buf, int in_buf_sz, int *read);


#endif // __LMS_PIPE_H__

