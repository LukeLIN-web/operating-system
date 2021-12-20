#ifndef SYSCALL_H
#define SYSCALL_H
#include "sched.h"
#include "put.h"
#include "stddef.h"
#include "slub.h"
#include "vm.h"

#define PROT_READ		0x00000001
#define PROT_WRITE  	0x00000002
#define PROT_EXEC		0x00000004

unsigned long sys_write(unsigned int fd, const char* buf, size_t count);

unsigned long sys_getpid();

#define __off_t unsigned long

unsigned long get_unmapped_area(size_t length);

void *do_mmap(struct mm_struct *mm, void *start, size_t length, int prot);

void *mmap (void *__addr, size_t __len, int __prot,
            int __flags, int __fd, __off_t __offset) ;

int munmap(void *start, size_t length) ;

int mprotect (void *__addr, size_t __len, int __prot) ;

int fork();
#endif