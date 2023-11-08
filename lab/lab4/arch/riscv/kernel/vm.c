#include"vm.h"
#include"put.h"
  
extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;

uint64_t current_addr;  // address of the last page table
uint64_t page_size = 0x1000; // 4KB
uint64_t addr_offset = 12;  // 12bit offset


void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm)
{
    uint64_t VPN0, VPN1, VPN2;  //three parts of VPN
    uint64_t valid = 1;  //last bit of page table entry 
    uint64_t count = 0;  //count total page table entry size
    uint64_t *pgtbl_1;  //the 2nd page table
    uint64_t *pgtbl_0;  //the 3th page table
    VPN0 = (va >> 30) & 0x1ff;  
    pgtbl[VPN0] = ((uint64_t)(pgtbl_1) >> addr_offset << 10) + 1;
    //first level page table entry stores the base address of the 2nd page table
    while (sz > 0)  //mapping the whole size
    {
        if (!(count & 0x1ff)) //check if the 1st table has been mapped
        {
            if (!(count & 0xfff))  //check if the 2nd table has been mapped
            {
                current_addr += page_size; //get a new unused addresss
                pgtbl_1 = (uint64_t *)current_addr;  //locate the 2nd page table to the new address
                pgtbl[VPN0] = ((uint64_t)(pgtbl_1) >> addr_offset << 10) + valid; //fill the page table entry
            }
            VPN1 = (va >> 21) & 0x1ff;  //use the same method to build the next level page table
            current_addr += page_size;
            pgtbl_0 = (uint64_t *)current_addr;
            pgtbl_1[VPN1] = ((uint64_t)(pgtbl_0) >> addr_offset << 10) + valid;
        }
        
        VPN2 = (va >> 12) & 0x1ff;   
        pgtbl_0[VPN2] = (pa >> addr_offset << 10) + (perm << 1) + valid;
        //the last level page table entry stores the PPN and the perm
        pa += page_size;  
        va += page_size;
        sz -= page_size;
        //move to the next page
        count++;
    }
}

void paging_init()
{
    uint64_t *pgtbl = &_end;
    current_addr = (uint64_t)&_end;
    // create_mapping(pgtbl, 0x80000000, 0x80000000, 0x1000000, 7); 
    // create_mapping(pgtbl, 0xffffffe000000000, 0x80000000, 0x1000000, 7); 
    // create_mapping(pgtbl, 0x10000000, 0x10000000, 0x1000, 7); 
    
    create_mapping(pgtbl,0xffffffe000000000,0x80000000,0x2000,11);// text
    create_mapping(pgtbl,0xffffffe000002000,0x80002000,0x1000,3);// rodata 
    create_mapping(pgtbl,0xffffffe000003000,0x80003000,0x1000000 - 0x3000,7);//else
    create_mapping(pgtbl,0x10000000,0x10000000, 0x1000,7);// UART
}
