#include "vm.h"
#include "put.h"
#include "slub.h"
#include "stddef.h"
#include "buddy.h"

extern char* _end;

struct kernel_mem_struct kmem_struct;

void create_table_dir(uint64 *tblptr, uint64 va, uint64 pa, int perm, int right) {
    tblptr = (uint64)tblptr - 0x80000000ul + page_offset;
    uint64 tbl_index = (va >> (uint64)right) & (uint64)0x1FF;
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            //  puti(((tblptr[tbl_index] >> 10) & (uint64)0xFFFFFFFFFFF) << 12); puts("\n");
            create_table_dir(((((tblptr[tbl_index]) >> 10) & (uint64)0xFFFFFFFFFFF) << 12) , va, pa, perm, right - 9);
        } else {
            // puts("! WRITE PA"); puti(((pa >> 12) << 10) | (perm << 1) | 1); puts("\n");
            tblptr[tbl_index] = ((pa >> 12) << 10) | (perm) | 1;
        }
    } else {
        if (right == 12) {
            // puts("! WRITE PA"); puti(((pa >> 12) << 10) | (perm << 1) | 1); puts("\n");
            tblptr[tbl_index] = ((pa >> 12) << 10) | (perm) | 1;
        } else {
            uint64 addr_top = kmalloc(PAGE_SIZE) - page_offset + 0x80000000ul;
            tblptr[tbl_index] = (((addr_top) >> 12) << 10) | 1;
            create_table_dir(addr_top, va, pa, perm, right - 9);
        }
    }
}

void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    // puts("va: "); puti(va); puts("\n");
    uint64 i = 0;
    uint64 va_aligned = va;// >> 12) << 12;
    uint64 pa_aligned = pa;// >> 12) << 12;
    for (i = 0; i < sz; i += 0x1000) {
        // puts("va + i: "); puti(va + i); puts("\n");
        // puts("pa + i: "); puti(pa + i); puts("\n");
        create_table_dir(pgtbl, va_aligned + i, pa_aligned + i, perm, 30);
    }
}

uint64 free_table_dir(uint64 *tblptr, uint64 va, uint64 right) {
    tblptr = (uint64)tblptr - 0x80000000ul + page_offset;
    uint64 tbl_index = (va >> (uint64)right) & (uint64)0x1FF;
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            return free_table_dir(((((tblptr[tbl_index]) >> 10) & (uint64)0xFFFFFFFFFFF) << 12), va, right - 9);
        } else {
            tblptr[tbl_index] ^= 1;
            return (tblptr[tbl_index] >> 10) << 12;
        }
    } else {
        return NULL;
    }
}

void free_page_tables(uint64 pgtbl, uint64 va, uint64 sz, int free_frame) {
    //`pagetable`为页表基地址，可从`task_struct`中读取或直接从`satp`获取
    //`va`为需要解除映射的虚拟地址的起始地址，`n`为需要解除映射的页的数量
    uint64 i = 0;
    for (i = 0; i < sz; i += 0x1000) {
        uint64 pa = free_table_dir(pgtbl, va + i, 30);
        if (pa != NULL && free_frame) {
            kfree(pa + kmem_struct.virtual_offset);
        }
    }
}

void protect_table_dir(uint64 *tblptr, uint64 va, int prot, uint64 right) {
    tblptr = (uint64)tblptr - 0x80000000ul + page_offset;
    uint64 tbl_index = (va >> (uint64)right) & (uint64)0x1FF;
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            protect_table_dir(((((tblptr[tbl_index]) >> 10) & (uint64)0xFFFFFFFFFFF) << 12), va, prot, right - 9);
        } else {
            tblptr[tbl_index] >>= 4;
            tblptr[tbl_index] = (tblptr[tbl_index] << 4) | prot | 1;
        }
    } else {
        return;
    }
}



void protect_page_tables(uint64 pgtbl, uint64 va, uint64 sz, int prot) {
    uint64 i = 0;
    uint64 va_aligned = va;// >> 12) << 12;
    for (i = 0; i < sz; i += 0x1000) {
        protect_table_dir(pgtbl, va_aligned + i, prot, 30);
    }
}

int has_table_dir(uint64 *tblptr, uint64 va, uint64 right) {
    tblptr = (uint64)tblptr - 0x80000000ul + page_offset;
    uint64 tbl_index = (va >> (uint64)right) & (uint64)0x1FF;
    
    if ((tblptr[tbl_index]) & 1) {
        if (((tblptr[tbl_index] >> 1) & 0x7) == 0) {
            return has_table_dir(((((tblptr[tbl_index]) >> 10) & (uint64)0xFFFFFFFFFFF) << 12), va, right - 9);
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

int has_page_tables(uint64 pgtbl, uint64 va) {
    return has_table_dir(pgtbl, va, 30);
}

uintptr_t paging_init(void) {
    uintptr_t pgtbl = kmalloc(PAGE_SIZE);//kmalloc返回的地址均设为虚拟地址. 进行一些变换到物理地址. 
    asm("la %0, text_start":"=r" (kmem_struct.text_start));
    asm("la %0, rodata_start":"=r" (kmem_struct.rodata_start));
    asm("la %0, data_start":"=r" (kmem_struct.data_start));
    kmem_struct.virtual_offset = 0xffffffe000000000 - 0x80000000;

    uintptr_t text_start = kmem_struct.text_start, rodata_start = kmem_struct.rodata_start, data_start = kmem_struct.data_start;
    uintptr_t virtual_offset = kmem_struct.virtual_offset;

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

