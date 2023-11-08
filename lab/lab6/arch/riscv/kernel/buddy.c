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
    unsigned long x, y, i;
    for (x = buddy.size, y = 1, i = 0; x > 0; x >>= 1, i += y, y <<= 1) {
        int j;
        for (j = 0; j < y; ++j) {
            ins_bitmap[i + j] = x;
        }
    }
    
    alloc_pages((((unsigned long)&_end - 0x80000000ul) >> 12));

}

void *alloc_pages(int n) {
    int i;
    for (i = 0; ; ++i) {
        if ((1 << i) >= n) {
            n = (1 << i);
            break;
        }
    }
    if (ins_bitmap[0] < n) {
        return 0;
    }
    i = 0;
    int full_size = buddy.size;
    unsigned t_bitmap;
    while (i < 2 * P_PAGE - 1) {
        if (ins_bitmap[i] == n && ins_bitmap[i] == full_size) {
            t_bitmap = ins_bitmap[i];
            ins_bitmap[i] = 0;
            int ii = i;
            while (ii > 0) {
                ii = parent(ii);
                ins_bitmap[ii] = ins_bitmap[rson(ii)] > ins_bitmap[lson(ii)] ? 
                        ins_bitmap[rson(ii)] : ins_bitmap[lson(ii)];
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
                    if (ins_bitmap[lson(ii)] == (t_full_size >> 1) && ins_bitmap[rson(ii)] == (t_full_size >> 1)) {
                        ins_bitmap[ii] = t_full_size;
                    } else {
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