#include"vm.h"
#include"put.h"
  
extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;


int page_count = 0;
int page_size = 4096;
void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm){
  uint64_t page_offset = 12;
    uint64_t valid = 1;
    for(uint64_t i = 0;i<sz/page_size; i++){
        uint64_t VPN[3];
        uint64_t Page_table_entry[3];
        uint64_t PPN;
        uint64_t *pgtbl_1;
        uint64_t *pgtbl_0;
        PPN = ((pa + i * page_size)&(0x7fff<<12))>>12;
        VPN[2] = ((va + i * page_size)&(0x1ff<<30))>>30;
        VPN[1] = ((va + i * page_size)&(0x1ff<<21))>>21;
        VPN[0] = ((va + i * page_size)&(0x1ff<<12))>>12;

        
        if(!pgtbl[VPN[2]]){
            pgtbl_1 = &_end + 0x1000 * page_count;
            page_count ++;
            Page_table_entry[2] = (0x1ffffff<<39) + ((uint64_t)pgtbl_1<<10) + (perm<<1) + valid;
            pgtbl[ VPN[2]] = Page_table_entry[2];
        }
        else{
            pgtbl_1 = (pgtbl[VPN[2]]>>10) << page_offset;
        }

        if(!pgtbl_1[VPN[1]]){
            pgtbl_0 = &_end + 0x1000 * page_count;
            page_count ++;
            Page_table_entry[1] = (0x1ffffff<<39) + ((uint64_t)pgtbl_0<<10) + (perm<<1) + valid;
            pgtbl_1[ VPN[1]] = Page_table_entry[1];
        }
        else{
            pgtbl_0 = (pgtbl[VPN[1]]>>10) << page_offset;
        }

        Page_table_entry[0] = (0x1ffffff<<39) + (PPN<<10) + (perm<<1) + valid;
        pgtbl_0[ VPN[0]] = Page_table_entry[0];
    }
	/*your code*/
}

void paging_init()
{
   uint64_t *pgtbl = &_end;
   create_mapping(pgtbl,0xffffffe000000000,0x8000000,0x7ffffff,7);
   create_mapping(pgtbl,0x8000000,0x8000000,0x7ffffff,7);
   create_mapping(pgtbl,0x1000000,0x1000000,page_size,7);
    /*your code*/
} 
