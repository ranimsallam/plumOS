#include "memory.h"

// set memory of ptr with c value for size bytes
void* memset(void *ptr, int c, size_t size)
{
    char* c_ptr = (char*) ptr;
    for (int i = 0; i < size; ++i) {
        c_ptr[i] = (char)c;
    }
    return ptr;
}

// compare count bytes of two pointers
// return 0 if equal, -1 if s1<s2 and 1 if s1>s2
int memcmp(void* s1, void* s2, int count)
{
    char* c1 = s1;
    char* c2 = s2;

    while ( count-- > 0 ) {
        
        if (*c1++ != *c2++) {
            return c1[-1] < c2[-1] ? -1 : 1;
        }
    }

    return 0;
}

void* memcpy(void* dest,  void* src, int len)
{
    char *d = dest;
    char *s = src;

    while(len--) {
        *d++ = *s++;
    }
    return dest;
}