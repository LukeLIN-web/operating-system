#include "buddy.h"
#include "sched.h"

#define P_PAGE 0x1000
#define parent(x) (((x + 1) >> 1) - 1)
#define lson(x) (((x + 1) << 1) - 1)
#define rson(x) ((x + 1) << 1)

struct buddy buddy;
unsigned ins_bitmap[2 * P_PAGE - 1];
unsigned long page_offset = 0x80000000ul;

extern unsigned long *_end;

void init_buddy_system(void) {
    buddy.size = P_PAGE;
    buddy.bitmap = ins_bitmap;
    unsigned long x = buddy.size;
    unsigned long y = 1;
    unsigned long i = 0;
    for (  ; x > 0; x >>= 1, i += y, y <<= 1) {  
        for (int j = 0; j < y; ++j) {
            ins_bitmap[i + j] = x;
        }
    }
    //由于kernel的代码占据了一部分空间，在初始化结束后，需要将这一部分的空间标记为已分配
    alloc_pages((((unsigned long)&_end - 0x80000000ul) >> 12));

}

void *alloc_pages(int n) {
    for (int offset = 0; ; ++offset) {
        if ((1 << offset) >= n) {
            n = (1 << offset);
            break;// 每次必须分配2^n个页面 
        }
    }
    if (ins_bitmap[0] < n) {
        return 0;
    }
    int i = 0;
    int full_size = buddy.size;
    unsigned t_bitmap;
    //查找恰好满足当前大小需求的节点
    while (i < 2 * P_PAGE - 1) {
        if (ins_bitmap[i] == n && ins_bitmap[i] == full_size) {
            t_bitmap = ins_bitmap[i];
            ins_bitmap[i] = 0;
            int ii = i;
            while (ii > 0) {
                ii = parent(ii);
                ins_bitmap[ii] = ins_bitmap[rson(ii)] > ins_bitmap[lson(ii)] ? 
                        ins_bitmap[rson(ii)] : ins_bitmap[lson(ii)];//更新bitmap
            }
            break;
        } else {
            if (ins_bitmap[lson(i)] >= n) {
                full_size >>= 1;
                i = lson(i);
            } else if (ins_bitmap[rson(i)] >= n) {
                full_size >>= 1;
                i = rson(i);
            }
        }
    }
    // 通过 index 找到对应 page X，再通过适当计算最终转化为VA
    unsigned long addr = (((i + 1) * t_bitmap - buddy.size) << 12) + page_offset;
    if (addr >= 0xffffffe000000000) {
        puts("[S] Buddy allocate addr: ");
        puti64(addr);
        puts("\n");
    }
    return addr;
}

void free_pages(void* x) {
    unsigned long index = x;
    index -= page_offset;
    index >>= 12;
    index += buddy.size;
    //通过 va 进行适当的转换，我们可以得到该页面对应的 bitmap index

    int i = 0;
    int full_size = buddy.size;
    while (i < 2 * P_PAGE - 1) {
        if ((i + 1) * ins_bitmap[i] == index) {
            if (ins_bitmap[i] == 0) {
                ins_bitmap[i] = full_size;
                int ii = i;
                int t_full_size = full_size;
                while (ii > 0) {
                    ii = parent(ii);
                    t_full_size <<= 1;
                    ////判断是否需要对祖先的左右两个孩子进行合并
                    if (ins_bitmap[lson(ii)] == (t_full_size >> 1) && ins_bitmap[rson(ii)] == (t_full_size >> 1)) {
                        ins_bitmap[ii] = t_full_size;//两个孩子都未被分配则合并
                    } else {
                        //否则不合并，修改其值为 左右两个孩子中最大物理连续页的数量
                        ins_bitmap[ii] = ins_bitmap[rson(ii)] > ins_bitmap[lson(ii)] ? 
                                ins_bitmap[rson(ii)] : ins_bitmap[lson(ii)];
                    }
                }
            } else {
                full_size >>= 1;
                i = lson(i);
            }
        } else {
            full_size >>= 1;
            i = rson(i);
        }
    }
}