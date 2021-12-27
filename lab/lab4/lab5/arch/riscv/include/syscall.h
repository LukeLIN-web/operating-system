#include "sched.h"
#include "put.h"


unsigned long sys_write(unsigned int fd, const char* buf, size_t count) {
    if (fd == 1) {
        puts(buf);
        return count;
    }
}

unsigned long sys_getpid() {
    return current->pid;
}