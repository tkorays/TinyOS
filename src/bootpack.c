#include "utilfunc.h"
#include "palette.h"
#include "graphic.h"
#include <stdio.h>
#include "bootpack.h"
#include "int.h"
#include "fifo.h"
#include "keyboard.h"
#include "mouse.h"
#include "memory.h"

void asm_inthandler21(void);
void asm_inthandler2c(void);

/**
 * 设置单个段属性段
 * @param sd 段描述符地址
 * @param limit 段大小
 * @param base 段的基地址
 * @param ar access right，段描述符的40~55bit
 */
void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar) {
    // 如果超过1M空间，则需要置Gbit
    if(limit > 0xfffff) {
        ar |= 0x8000; // G_bit = 1
        limit /= 0x1000;
    }

    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_heigh = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    sd->base_high = (base >> 24) & 0xff; 
}

void set_gatedesc(struct GATE_DESCRIPTOR* gd, int offset, int selector, int ar) {
    gd->offset_low = offset & 0xffff;
    gd->selector = selector;
    gd->dw_count = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high = (offset >> 16) & 0xffff;
}

void init_gdtidt(void) {
    struct SEGMENT_DESCRIPTOR*  gdt = (struct SEGMENT_DESCRIPTOR*)0x00270000;
    struct GATE_DESCRIPTOR*     idt = (struct GATE_DESCRIPTOR*)0x0026f800;
    int i;

    for(i = 0; i < 8192; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }

    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);

    load_gdtr(0xffff, 0x00270000);

    for(i = 0; i < 256; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    load_idtr(0x7ff, 0x0026f800);

    set_gatedesc(idt + 0x21, (int)asm_inthandler21, 2*8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int)asm_inthandler2c, 2*8, AR_INTGATE32);
}

#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000

unsigned int memtest(unsigned int start, unsigned int end) {
    /**
     * 内存检查使用的原理：写入值后读取，如果内存不存在则读出的值不正确
     * 486以上要禁用缓存，缓存会影响该方法导致失效
     */
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT;
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if((eflg & EFLAGS_AC_BIT) != 0) {
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT;
    io_store_eflags(eflg);

    // 禁用缓存
    if(flg486 != 0) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if(flg486 != 0) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        store_cr0(cr0);
    }

    return i;
}

extern struct FIFO8 keyinfo;
extern struct FIFO8 mouseinfo;

void HariMain(void) {
    int i;
    char s[128], keybuf[32], mousebuf[128], mcursor[256];
    int c, mx = 0, my = 0;
    unsigned int memsize = 0;
    //unsigned char* p = (unsigned char*)0xa0000;
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
    struct MOUSE_DEC mdec;

    struct MEMMAN* memman = (struct MEMMAN*)MEMMAN_ADDR;

    init_gdtidt();
    init_pic();
    io_sti();
    
    fifo8_init(&keyinfo, 32, keybuf);
    fifo8_init(&mouseinfo, 128, mousebuf);

    io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);

    init_palette();


    boxfill8((unsigned char*)binfo->vram, binfo->scrnx, 1, 0, 0, binfo->scrnx, binfo->scrny);

    init_system_ui((unsigned char*)binfo->vram, binfo->scrnx, binfo->scrny);
    init_mouse_cursor8(mcursor, COLOR8_008484);
    
    //boxfill8(p, 320, 5, 20, 20, 120, 120);
    //boxfill8(p, 320, 8, 70, 70, 170, 170);

    memsize = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memsize - 0x00400000);

    sprintf(s, "memsize: %dM, free: %dkB", memsize / (1024*1024), memman_total(memman)/1024);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 100, COLOR8_FFFFFF, s);


    init_keyboard();
    enable_mouse(&mdec);
    

    for(;;) {
        io_cli();
        if(fifo8_status(&keyinfo) + fifo8_status(&mouseinfo) == 0) {
            io_stihlt();
        } else {
            if(fifo8_status(&keyinfo) != 0) {
                c  = fifo8_get(&keyinfo);
                io_sti();
                sprintf(s, "%02X", c);

                boxfill8((unsigned char*)binfo->vram, binfo->scrnx, COLOR8_000000, 0, 16, 15, 31);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COLOR8_FFFFFF, s);
            } else if(fifo8_status(&mouseinfo) != 0) {
                c  = fifo8_get(&mouseinfo);
                io_sti();

                if(mouse_decode(&mdec, c) != 0) {
                    sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                    if((mdec.btn & 0x01) != 0) {
                        s[1] = 'L';
                    }
                    if((mdec.btn & 0x02) != 0) {
                        s[3] = 'R';
                    }
                    if((mdec.btn & 0x04) != 0) {
                        s[2] = 'C';
                    }

                    boxfill8((unsigned char*)binfo->vram, binfo->scrnx, COLOR8_000000, 32, 16, 32+15*8-1, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COLOR8_FFFFFF, s);

                    boxfill8(binfo->vram, binfo->scrnx, COLOR8_008484, mx, my, mx+15, my+15);
                    mx += mdec.x;
                    my += mdec.y;

                    if(mx < 0) mx = 0;
                    if(my < 0) my = 0;
                    if(mx > binfo->scrnx - 16) mx = binfo->scrnx - 16;
                    if(mx > binfo->scrny - 16) my = binfo->scrny - 16;
	                putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
                }


            }
        }
    }
}
