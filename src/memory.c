#include "memory.h"

void memman_init(struct MEMMAN* man) {
    man->frees      = 0;
    man->maxfrees   = 0;
    man->lostsize   = 0;
    man->losts      = 0;
}

unsigned int memman_total(struct MEMMAN* man) {
    unsigned int i, t = 0;
    for(i = 0; i < man->frees; i++) {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN* man, unsigned int size) {
    unsigned int i, a;
    for(i = 0; i < man->frees; i++) {
        if(man->free[i].size >= size) {
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if(man->free[i].size == 0) {
                man->frees--;
                for(; i < man->frees; i++) {
                    man->free[i] = man->free[i+1];
                }
            }
            return a;
        }
    }
    return 0;
}

int memman_free(struct MEMMAN* man, unsigned int addr, unsigned size) {
    int i, j;

    for(i = 0; i < man->frees; i++) {
        if(man->free[i].addr > addr) {
            break;
        }
    }

    /**
     * man->free[i-1].addr < addr < man->free[i].addr
     */

    if(i > 0) {
        // 前面一片内存和这篇内存可以连起来
        if(man->free[i-1].addr + man->free[i-1].size == addr) {
            man->free[i-1].size += size;

            // 后面一片内存也可以和这边内存连起来
            if(addr + size == man->free[i+1].addr) {
                man->free[i-1].size += man->free[i+1].size;

                man->frees--;
                for(; i < man->frees; i++) {
                    man->free[i] = man->free[i+1];
                }
            }
            return 0;
        }
    }

    if(i < man->frees) {
        // 可以和后面的内存连起来
        if(addr + size == man->free[i+1].addr) {
            man->free[i+1].addr = addr;
            man->free[i+1].size += size;
            return 0;
        }
    }

    // 前后均不能相连，则创建一个新的控制块
    if(man->frees < MEMMAN_FREES) {
        for(j = man->frees; j > i; j--) {
            man->free[j] = man->free[j-1];
        }
        man->frees++;
        if(man->maxfrees < man->frees) {
            man->maxfrees = man->frees;
        }

        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;
    }

    /* 操作失败，这些内存再也不能被使用 */
    man->losts++;
    man->lostsize += size;
    return -1;
}

unsigned int memman_alloc_4k(struct MEMMAN* man, unsigned int size) {
    unsigned int a;
    size = (size + 0xfff) & 0xfffff000;
    a = memman_alloc(man, size);
    return a;
}

int memman_free_4k(struct MEMMAN* man, unsigned int addr, unsigned int size) {
    int i;
    size = (size + 0xfff) & 0xfffff000;
    i = memman_free(man, addr, size);
    return i;
}

