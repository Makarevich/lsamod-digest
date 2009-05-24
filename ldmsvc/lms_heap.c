
#include <windows.h>
#include "lms_heap.h"

#ifdef OWN_HEAP

static HANDLE       heap;

int heap_start(){
    heap = HeapCreate(/* HEAP_NO_SERIALIZE */0 , 4000, 0);
    return heap != NULL;
}

int heap_stop(){
    return HeapDestroy(heap);
}

void* heap_alloc(int sz){
    return HeapAlloc(heap, 0, sz);
}

int heap_free(void* p){
    return HeapFree(heap, 0, p);
}

#else

int heap_start(){
    return TRUE;
}

int heap_stop(){
    return TRUE;
}

void* heap_alloc(int sz){
    return HeapAlloc(GetProcessHeap(), 0, sz);
}

int heap_free(void* p){
    return HeapFree(GetProcessHeap(), 0, p);
}

#endif


