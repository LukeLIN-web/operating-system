#ifndef _VM_H
#define _VM_H

// void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa, unsigned long sz, int perm);
#include "sched.h"
extern unsigned long addr_top;

struct kernel_mem_struct {
    unsigned long text_start, rodata_start, data_start;
    unsigned long virtual_offset;
};

extern struct kernel_mem_struct kmem_struct;

unsigned long paging_init(void);
void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa, unsigned long sz, int perm);
void free_page_tables(unsigned long pagetable, unsigned long va, unsigned long sz, int free_frame);
void protect_page_tables(unsigned long pgtbl, unsigned long va, unsigned long sz, int prot);
int has_page_tables(unsigned long pgtbl, unsigned long va);


#endif /* _VM_H */