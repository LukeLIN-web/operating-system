#include "vm.h"
#include "put.h"

extern char* _end;

unsigned long addr_top = 0;
struct kernel_mem_struct kmem_struct;

void create_table_dir(unsigned long *tblptr, unsigned long va, unsigned long pa, int perm, int right) {
    unsigned long tbl_index = (va >> (unsigned long)right) & (unsigned long)0x1FF;
    // puts("tbl_index: "); puti(tbl_index); puts("\n");
    // puts("right: "); puti(right); puts("\n");
    // puts("va: "); puti(va); puts("\n");
    // puts("pa: "); puti(pa); puts("\n");
    // puts("tblptr: "); puti(tblptr); puts("\n");
    // puts("tblptr[tbl_index]: "); puti(tblptr[tbl_index]); puts("\n");
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            //puti(((tblptr[tbl_index] >> 10) & (unsigned long)0xFFFFFFFFFFF) << 12); puts("\n");
            create_table_dir(((tblptr[tbl_index] >> 10) & (unsigned long)0xFFFFFFFFFFF) << 12, va, pa, perm, right - 9);
        } else {
            // puts("! WRITE PA"); puti(((pa >> 12) << 10) | (perm << 1) | 1); puts("\n");
            tblptr[tbl_index] = ((pa >> 12) << 10) | (perm << 1) | 1;
        }
    } else {
        if (right == 12) {
            // puts("! WRITE PA"); puti(((pa >> 12) << 10) | (perm << 1) | 1); puts("\n");
            tblptr[tbl_index] = ((pa >> 12) << 10) | (perm << 1) | 1;
        } else {
            tblptr[tbl_index] = ((addr_top >> 12) << 10) | 1;
            addr_top += 0x1000;
            create_table_dir(addr_top - 0x1000, va, pa, perm, right - 9);
        }
    }
}

void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa, unsigned long sz, int perm) {
    // puts("va: "); puti(va); puts("\n");
    unsigned long i = 0;
    unsigned long va_aligned = va;    
    unsigned long pa_aligned = pa;
    for (i = 0; i < sz; i += 0x1000) {
        // puts("va + i: "); puti(va + i); puts("\n");
        // puts("pa + i: "); puti(pa + i); puts("\n");
        create_table_dir(pgtbl, va_aligned + i, pa_aligned + i, perm, 30);
    }
}

void paging_init(void) {
    addr_top = (unsigned long)&_end;
    // puti(addr_top); puts("\n");
    unsigned long pgtbl = addr_top;
    addr_top += 0x1000;
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

    create_mapping(pgtbl, virtual_offset + text_start, text_start, rodata_start - text_start, 5);
    create_mapping(pgtbl, virtual_offset + rodata_start, rodata_start, data_start - rodata_start, 1);
    create_mapping(pgtbl, virtual_offset + data_start, data_start, 0x1000000 - (data_start - text_start), 3);
    // puts("-------\n");
    create_mapping(pgtbl, text_start, text_start, rodata_start - text_start, 5);
    create_mapping(pgtbl, rodata_start, rodata_start, data_start - rodata_start, 1);
    create_mapping(pgtbl, data_start, data_start, 0x1000000 - (data_start - text_start), 3);
    // puts("-------\n");
    create_mapping(pgtbl, 0xffffffdf90000000, 0x10000000, 0x1000, 3);
    // puti(addr_top); puts("-------\n");
}