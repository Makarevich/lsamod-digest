
#ifndef __SHMEM_H__
#define __SHMEM_H__

#define LDROP_NOP           0
#define LDROP_LOADLIB       1
#define LDROP_FREELIB       2
#define LDROP_CALLPROC      3

typedef struct {
    int         op;
    char        data[500];
} shmem_ldr_t;




int shmem_create(char** out_buffer, HANDLE *out_mutex);
int shmem_open(char** out_buffer, HANDLE *out_mutex);
int shmem_close(char* buf, HANDLE mutex);

#endif // __SHMEM_H__

