#include "vm.h"
#include "put.h"

extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;

uint64_t cur; // address of the last page table
int page_count = 1;
int page_size = 4096;
uint64_t page_offset = 12;
uint64_t valid = 1;
void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm)
{
    // puts("begin create mapping\n");
    int index1, index2, index3;
    static int flag = 0;
    static uint64_t *putbl;
    uint64_t *pmtbl;
    index1 = (va & 0x7FC0000000) >> 30;
    pgtbl[index1] = ((uint64_t)(putbl) >> 12 << 10) + 1;
    while (sz > 0)
    {
        if ((flag & 0x1ff) == 0)
        {
            if ((flag & 0xfff) == 0)
            {
                cur += 0x1000;
                putbl = (uint64_t *)cur;
                pgtbl[index1] = ((uint64_t)(putbl) >> 12 << 10) + 1;
            }
            index2 = (va & 0x3FE00000) >> 21;
            cur += 0x1000;
            pmtbl = (uint64_t *)cur;
            putbl[index2] = ((uint64_t)(pmtbl) >> 12 << 10) + 1;
        }

        index3 = (va & 0x1FF000) >> 12;
        pmtbl[index3] = (pa >> 12 << 10) + 1 + (perm << 1);
        pa += 0x1000;
        va += 0x1000;
        sz -= 0x1000;
        flag++;
    }
    //     puts("\nnew entry\n")
    //     // puti(page_count);
    //     puts("\nexist entry   \n");
    //     puts("\nputullHex(VPN[2]) =    \n");
    //     putullHex(VPN[2]);
    //     puts("\npgtbl_1 = (pgtbl[VPN[2]]>>10) << page_offset; =    \n");
    //     putullHex(pgtbl_1);
    // puts("\n!pgtbl[VPN[2]]  page table finished\n");
    /*your code*/
}

void paging_init()
{
    uint64_t *pgtbl = &_end;
    cur = (uint64_t)&_end;
    create_mapping(pgtbl, 0xffffffe000000000, 0x80000000, 0x1000000, 7);
    // puts("finished create mapping first\n");
    create_mapping(pgtbl, 0x80000000, 0x80000000, 0x1000000, 7);
    // puts("finished create mapping second");
    create_mapping(pgtbl, 0x10000000, 0x10000000, page_size, 7);
    // puts("finished create mapping");
    puts("\npaging_init finished  \n");
    /*your code*/
}
