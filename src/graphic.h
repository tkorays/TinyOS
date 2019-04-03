#ifndef __GRAPHIC_H__
#define __GRAPHIC_H__

void init_system_ui(unsigned char* vram, int pitch, int height);
void boxfill8(unsigned char* vram, int pitch, unsigned char c, int left, int top, int right, int bottom);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);

#endif /* __GRAPHIC_H__ */
