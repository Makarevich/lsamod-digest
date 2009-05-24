

#ifndef __LMS_HEAP_H__
#define __LMS_HEAP_H__

int heap_start();
int heap_stop();

void* heap_alloc(int sz);
int heap_free(void* p);

#endif // __LMS_HEAP_H__




