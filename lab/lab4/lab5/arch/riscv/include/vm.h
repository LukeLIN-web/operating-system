#ifndef _VM_H
#define _VM_H

// void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa, unsigned long sz, int perm);

extern unsigned long addr_top;

struct kernel_mem_struct {
    unsigned long text_start, rodata_start, data_start;
    unsigned long virtual_offset;
};

extern struct kernel_mem_struct kmem_struct;

void paging_init(void);

#endif /* _VM_H */