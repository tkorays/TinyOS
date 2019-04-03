
#include "graphic.h"
#include "palette.h"

void boxfill8(unsigned char* vram, int pitch, unsigned char c, int left, int top, int right, int bottom) {
    int x, y;
    for(y = top; y <= bottom; y++) {
        for(x = left; x <= right; x++) {
            vram[y*pitch + x] = c;
        }
    }
    return ;
}

void init_system_ui(unsigned char* vram, int pitch, int height) {
    boxfill8(vram, pitch, COLOR8_008484, 0, 0, pitch - 1, height - 29);

    boxfill8(vram, pitch, COLOR8_C6C6C6, 0, height - 28, pitch - 1, height - 28);
    boxfill8(vram, pitch, COLOR8_FFFFFF, 0, height - 27, pitch - 1, height - 27);
    boxfill8(vram, pitch, COLOR8_C6C6C6, 0, height - 26, pitch - 1, height - 1);


    boxfill8(vram, pitch, COLOR8_FFFFFF, 3, height - 24, 59, height - 24);
    boxfill8(vram, pitch, COLOR8_FFFFFF, 2, height - 24, 2, height - 4);
    boxfill8(vram, pitch, COLOR8_848484, 3, height - 4, 59, height - 4);
    boxfill8(vram, pitch, COLOR8_848484, 59, height - 23, 59, height - 5);
    boxfill8(vram, pitch, COLOR8_000000, 2, height - 3, 59, height - 3);
    boxfill8(vram, pitch, COLOR8_000000, 60, height - 24, 60, height - 3);

    boxfill8(vram, pitch, COLOR8_848484, pitch - 47, height - 24, pitch - 4, height - 24);
    boxfill8(vram, pitch, COLOR8_848484, pitch - 47, height - 23, pitch - 47, height - 4);
    boxfill8(vram, pitch, COLOR8_FFFFFF, pitch - 47, height - 3, pitch - 4, height - 3);
    boxfill8(vram, pitch, COLOR8_FFFFFF, pitch - 3, height - 24, pitch - 3, height - 3);
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

void init_mouse_cursor8(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COLOR8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COLOR8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}