#include"vm.h"
#include"put.h"
  
extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;


int page_count = 1;
int page_size = 4096;
uint64_t page_offset = 12;
uint64_t valid = 1;
void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm){
    puts("begin create mapping\n");
    uint64_t VPN[3];
    uint64_t Page_table_entry[3];
    uint64_t PPN;
    uint64_t *pgtbl_1;
    uint64_t *pgtbl_0;
    for(uint64_t i = 0;i < sz/page_size; i++){
        PPN = ((pa + i * page_size)&(0x7fff<<12))>>12;
        VPN[2] = ((va + i * page_size) >>30)&0x1ff;
        VPN[1] = ((va + i * page_size)>>21)&0x1ff;
        VPN[0] = ((va + i * page_size)>>12)&0x1ff;
        // puts("VPN[0]\n");
        // putullHex(VPN[0]);
        // puts("VPN[1]\n");
        // putullHex(VPN[1]);
        // putullHex(VPN[2]);

        if(!pgtbl[VPN[2]]){
            puts("\nnew entry\n");
            pgtbl_1 =(uint64_t *)( &_end + 0x1000 * page_count);
            page_count ++;
            // Page_table_entry[2] = (0x1ffffff<<39) + ((uint64_t)pgtbl_1<<10) + (perm<<1) + valid;
            // puti(page_count);
            Page_table_entry[2] = (((uint64_t)pgtbl_1>>12)<<10)  + valid;
            pgtbl[VPN[2]] = Page_table_entry[2];
        }
        else{
            puts("\nexist entry   \n");
            puts("\nputullHex(VPN[2]) =    \n");
            putullHex(VPN[2]);
            pgtbl_1 = (uint64_t *)((pgtbl[VPN[2]]>>10) << page_offset);
            puts("\npgtbl_1 = (pgtbl[VPN[2]]>>10) << page_offset; =    \n");
            putullHex(pgtbl_1);
        }
        puts("\n!pgtbl[VPN[2]]  page table finished\n");

        if(!pgtbl_1[VPN[1]]){
            pgtbl_0 = (uint64_t *)(&_end + 0x1000 * page_count);
            // pgtbl_0 = 0x8000000 + 0x1000 * page_count;
            page_count ++; // page count 或许不能全局变量.
            //Page_table_entry[1] = ((uint64_t)0x1ffffff<<39) + (((uint64_t)pgtbl_1>>12)<<10)  + valid;
            Page_table_entry[1] =  (((uint64_t)pgtbl_0>>12)<<10)  + valid;
            pgtbl_1[VPN[1]] = Page_table_entry[1];
        }
        else{
            pgtbl_0 =  (uint64_t *)((pgtbl[VPN[1]]>>10) << page_offset);
        }
        puts("!pgtbl[VPN[1]]  page table finished\n");
        //Page_table_entry[0] = ((uint64_t)0x1ffffff<<39) + ((PPN>>12)<<10) + (perm<<1) + valid;  
        Page_table_entry[0] = ((pa>>12)<<10) + (perm<<1) + valid;
        pgtbl_0[ VPN[0]] = Page_table_entry[0];
    }
	/*your code*/
}

void paging_init()
{
   uint64_t *pgtbl = &_end;
   create_mapping(pgtbl,0xffffffe000000000,0x8000000,0x1000000,7);
   puts("finished create mapping first\n");
   create_mapping(pgtbl,0x8000000,0x8000000,0x1000000,7);
   puts("finished create mapping second");
   create_mapping(pgtbl,0x1000000,0x1000000,page_size,7);
   puts("finished create mapping");
   uint64_t testva=0x10000230;
    testva=testva&0x7fffffffff;
    testva=testva>>12;
    puts("1");
    uint64_t vpn2,vpn1,vpn0;
    vpn0=testva&0x1ff;
    vpn1=(testva>>9)&0x1ff;
    vpn2=(testva>>18)&0x1ff;
    puts("0");
    uint64_t* p =pgtbl; 
    putullHex((uint64_t)p);
    puts("\n1\n");
    p=(p[vpn2]>>10)<<12;
    putullHex((uint64_t)p);
    puts("2\n");
    p=(p[vpn1]>>10)<<12;
    putullHex((uint64_t)p);
    puts("3\n");
    p=(p[vpn0]>>10)<<12;
    puts("4\n");
    putullHex((uint64_t)p);
    puts("\n1\n"); 
    /*your code*/
} 
