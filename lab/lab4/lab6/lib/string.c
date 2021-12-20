#include "string.h"

void* memset(void *dst, int c, uint n) {
    int i = 0;
    for (i = 0; i < n; ++i) {
        ((char*)dst)[i] = c;
    }
    return dst;
}