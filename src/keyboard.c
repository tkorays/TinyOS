#include "keyboard.h"
#include "bootpack.h"
#include "utilfunc.h"
#include "int.h"
#include "fifo.h"

struct FIFO8 keyinfo;

void wait_KBC_sendready(void) {
    for(;;) {
        if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) break;
    }
}

// 初始化键盘控制电路
void init_keyboard(void) {
    wait_KBC_sendready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
}


void inthandler21(int* esp) {
    unsigned char c;
    io_out8(PIC0_OCW2, 0x61);
    c = io_in8(PORT_KEYDAT);

    fifo8_put(&keyinfo, c);
    return;
}