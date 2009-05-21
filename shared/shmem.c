#include <windows.h>
#include "shmem.h"
#include "dout.h"
#include "utils.h"

#define SHMEM_MAPPING_NAME          "lsamod_shared_memory"
#define SHMEM_MUTEX_NAME            "lsamod_shared_mutex"

#define IFAPICALLFAILED(api, args, ret_var)    if((ret_var = api args) == NULL){ DOUTGLE("shmem", #api)
#define ENDIF   }


static HANDLE           sh_mapping = NULL;


int shmem_create(char** out_buffer, HANDLE *out_mutex){
    HANDLE                  mutex;
    char                    *buf;
    SECURITY_ATTRIBUTES     sa;
    SECURITY_DESCRIPTOR     sd;
    
    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)){
        DOUTGLE("shmem", "InitializeSecurityDescriptor");
        return 0;
    }

    if(!SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE)){
        DOUTGLE("shmem", "SetSecurityDescriptorDacl");
        return 0;
    }

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    IFAPICALLFAILED(CreateMutex, (NULL, FALSE, SHMEM_MUTEX_NAME), mutex);
        return 0;
    ENDIF;

    if(sh_mapping != NULL) CloseHandle(sh_mapping);

    IFAPICALLFAILED(CreateFileMapping, (INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, 4096, SHMEM_MAPPING_NAME), sh_mapping);
        CloseHandle(mutex);
        return 0;
    ENDIF;

    IFAPICALLFAILED(MapViewOfFile, (sh_mapping, FILE_MAP_WRITE, 0, 0, 0), buf);
        CloseHandle(sh_mapping);
        CloseHandle(mutex);
        return 0;
    ENDIF;

    *out_buffer = buf;
    *out_mutex = mutex;

    return 1;
}



int shmem_open(char** out_buffer, HANDLE *out_mutex){
    HANDLE      mapping2, mutex;
    char        *buf;

    IFAPICALLFAILED(OpenMutex, (SYNCHRONIZE|MUTEX_MODIFY_STATE, FALSE, SHMEM_MUTEX_NAME), mutex);
        return 0;
    ENDIF;

    IFAPICALLFAILED(OpenFileMapping, (FILE_MAP_WRITE, FALSE, SHMEM_MAPPING_NAME), mapping2);
        CloseHandle(mutex);
        return 0;
    ENDIF;

    IFAPICALLFAILED(MapViewOfFile, (mapping2, FILE_MAP_WRITE, 0, 0, 0), buf);
        CloseHandle(mapping2);
        CloseHandle(mutex);
        return 0;
    ENDIF;

    CloseHandle(mapping2);

    *out_buffer = buf;
    *out_mutex = mutex;

    return 1;
}

int shmem_close(char* buf, HANDLE mutex){
    UnmapViewOfFile(buf);
    CloseHandle(mutex);

    if(sh_mapping != NULL) CloseHandle(sh_mapping);

    return 1;
}

