#include "vm.h"
#include "put.h"
#include "slub.h"
#include "stddef.h"
#include "buddy.h"

extern char* _end;

struct kernel_mem_struct kmem_struct;

void create_table_dir(unsigned long *tblptr, unsigned long va, unsigned long pa, int perm, int right) {
    tblptr = (unsigned long)tblptr - 0x80000000ul + page_offset;
    unsigned long tbl_index = (va >> (unsigned long)right) & (unsigned long)0x1FF;
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            // puts("$$$$$1%%%%%%, "); puti(((tblptr[tbl_index] >> 10) & (unsigned long)0xFFFFFFFFFFF) << 12); puts("\n");
            create_table_dir(((((tblptr[tbl_index]) >> 10) & (unsigned long)0xFFFFFFFFFFF) << 12) , va, pa, perm, right - 9);
        } else {
            // puts("! WRITE PA"); puti(((pa >> 12) << 10) | (perm << 1) | 1); puts("\n");
            tblptr[tbl_index] = ((pa >> 12) << 10) | (perm) | 1;
        }
    } else {
        if (right == 12) {
            // puts("! WRITE PA"); puti(((pa >> 12) << 10) | (perm << 1) | 1); puts("\n");
            tblptr[tbl_index] = ((pa >> 12) << 10) | (perm) | 1;
        } else {
            unsigned long addr_top = kmalloc(PAGE_SIZE) - page_offset + 0x80000000ul;
            tblptr[tbl_index] = (((addr_top) >> 12) << 10) | 1;
            // puts("$$$$$2%%%%%%\n");
            create_table_dir(addr_top, va, pa, perm, right - 9);
        }
    }
}

void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa, unsigned long sz, int perm) {
    // puts("va: "); puti(va); puts("\n");
    unsigned long i = 0;
    unsigned long va_aligned = va;// >> 12) << 12;
    unsigned long pa_aligned = pa;// >> 12) << 12;
    for (i = 0; i < sz; i += 0x1000) {
        
        // puts("va + i: "); puti(va + i); puts("\n");
        // puts("pa + i: "); puti(pa + i); puts("\n");
        create_table_dir(pgtbl, va_aligned + i, pa_aligned + i, perm, 30);
    }
}

unsigned long free_table_dir(unsigned long *tblptr, unsigned long va, unsigned long right) {
    tblptr = (unsigned long)tblptr - 0x80000000ul + page_offset;
    unsigned long tbl_index = (va >> (unsigned long)right) & (unsigned long)0x1FF;
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            return free_table_dir(((((tblptr[tbl_index]) >> 10) & (unsigned long)0xFFFFFFFFFFF) << 12), va, right - 9);
        } else {
            tblptr[tbl_index] ^= 1;
            return (tblptr[tbl_index] >> 10) << 12;
        }
    } else {
        return NULL;
    }
}

void free_page_tables(unsigned long pgtbl, unsigned long va, unsigned long sz, int free_frame) {
    unsigned long i = 0;
    unsigned long va_aligned = va;// >> 12) << 12;
    for (i = 0; i < sz; i += 0x1000) {
        unsigned long pa = free_table_dir(pgtbl, va_aligned + i, 30);
        if (pa != NULL && free_frame) {
            kfree(pa + kmem_struct.virtual_offset);
        }
    }
}

void protect_table_dir(unsigned long *tblptr, unsigned long va, int prot, unsigned long right) {
    tblptr = (unsigned long)tblptr - 0x80000000ul + page_offset;
    unsigned long tbl_index = (va >> (unsigned long)right) & (unsigned long)0x1FF;
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            protect_table_dir(((((tblptr[tbl_index]) >> 10) & (unsigned long)0xFFFFFFFFFFF) << 12), va, prot, right - 9);
        } else {
            tblptr[tbl_index] >>= 4;
            tblptr[tbl_index] = (tblptr[tbl_index] << 4) | prot | 1;
        }
    } else {
        return;
    }
}



void protect_page_tables(unsigned long pgtbl, unsigned long va, unsigned long sz, int prot) {
    unsigned long i = 0;
    unsigned long va_aligned = va;// >> 12) << 12;
    for (i = 0; i < sz; i += 0x1000) {
        protect_table_dir(pgtbl, va_aligned + i, prot, 30);
    }
}

int has_table_dir(unsigned long *tblptr, unsigned long va, unsigned long right) {
    tblptr = (unsigned long)tblptr - 0x80000000ul + page_offset;
    unsigned long tbl_index = (va >> (unsigned long)right) & (unsigned long)0x1FF;
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            return has_table_dir(((((tblptr[tbl_index]) >> 10) & (unsigned long)0xFFFFFFFFFFF) << 12), va, right - 9);
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

int has_page_tables(unsigned long pgtbl, unsigned long va) {
    return has_table_dir(pgtbl, va, 30);
}

unsigned long paging_init(void) {
    unsigned long pgtbl = kmalloc(PAGE_SIZE);
    asm("la %0, text_start":"=r" (kmem_struct.text_start));
    asm("la %0, rodata_start":"=r" (kmem_struct.rodata_start));
    asm("la %0, data_start":"=r" (kmem_struct.data_start));
    kmem_struct.virtual_offset = 0xffffffe000000000 - 0x80000000;

    unsigned long text_start = kmem_struct.text_start, rodata_start = kmem_struct.rodata_start, data_start = kmem_struct.data_start;
    unsigned long virtual_offset = kmem_struct.virtual_offset;

    // puti(text_start); puts("\n");
    // puti(rodata_start); puts("\n");
    // puti(data_start); puts("\n");
    // puti(virtual_offset); puts("\n");

    create_mapping(pgtbl, virtual_offset + text_start, text_start, rodata_start - text_start, 5 << 1);
    create_mapping(pgtbl, virtual_offset + rodata_start, rodata_start, data_start - rodata_start, 1 << 1);
    create_mapping(pgtbl, virtual_offset + data_start, data_start, 0x1000000 - (data_start - text_start), 3 << 1);
    // puts("-------\n");
    create_mapping(pgtbl, text_start, text_start, rodata_start - text_start, 5 << 1);
    create_mapping(pgtbl, rodata_start, rodata_start, data_start - rodata_start, 1 << 1);
    create_mapping(pgtbl, data_start, data_start, 0x1000000 - (data_start - text_start), 3 << 1);
    // puts("-------\n");
    create_mapping(pgtbl, 0xffffffdf90000000, 0x10000000, 0x1000, 3 << 1);
    page_offset = 0xffffffe000000000;
    return pgtbl;
}

