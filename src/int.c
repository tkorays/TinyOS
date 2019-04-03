#include "int.h"
#include "utilfunc.h"
#include "bootpack.h"
#include "graphic.h"
#include "palette.h"
#include "fifo.h"
#include <stdio.h>

#define PORT_KEYDAT 0x0060

void init_pic(void) {
    io_out8(PIC0_IMR,  0xff); // 禁止所有中断
    io_out8(PIC1_IMR,  0xff);

    io_out8(PIC0_ICW1, 0x11  ); 
	io_out8(PIC0_ICW2, 0x20  );
	io_out8(PIC0_ICW3, 1 << 2);
	io_out8(PIC0_ICW4, 0x01  );

	io_out8(PIC1_ICW1, 0x11  );
	io_out8(PIC1_ICW2, 0x28  );
	io_out8(PIC1_ICW3, 2     );
	io_out8(PIC1_ICW4, 0x01  );

	io_out8(PIC0_IMR,  0xfb  );
	io_out8(PIC1_IMR,  0xff  );
}

