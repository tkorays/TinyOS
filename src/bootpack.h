#ifndef __BOOT_PACK_H__
#define __BOOT_PACK_H__


#define PORT_KEYDAT				0x0060
#define PORT_KEYCMD				0x0064
#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47
#define KEYCMD_SENDTO_MOUSE     0xd4
#define MOUSECMD_ENABLE         0xf4


// 系统启动后按照此格式保存相关信息
struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};
#define ADR_BOOTINFO	0x00000ff0

/**
 * 数据段描述符
 * BASE(24~31) || G B 0 AVI LIMIT(16~19) ||| 1 DPI S=1 TYPE || BASE(16~23)
 * BASE(0~15) ||| LIMIT(0~15)
 * 
 * 地址32bit，limit 20位，Gbit=0时可表示1M，Gbit=1时可表示4kbyte*1M=4G
 * B/D标志，取决于代码段还是数据段
 * S 系统标志
 */
struct SEGMENT_DESCRIPTOR {
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_heigh, base_high;
};

/**
 * 中断描述符
 * 
 * OFFSET(16~31) ||| P DPL 0 1 1 1 0 0 0 0 reserved
 * SELECTOR ||| OFFSET(0~15)
 * 
 */
struct GATE_DESCRIPTOR {
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};


#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e


#endif /* __BOOT_PACK_H__ */
