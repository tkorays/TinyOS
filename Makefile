TOOLPATH = d:/z_tools/
INCPATH  = d:/z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = $(TOOLPATH)haribote/haribote.rul
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
MAKEFONT = $(TOOLPATH)makefont.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
COPY     = copy
DEL      = del

objects = palette.obj utilfunc.obj bootpack.obj graphic.obj hankaku.obj int.obj fifo.obj \
	keyboard.obj mouse.obj memory.obj

ipl.bin: src/asm/ipl.nas Makefile
	$(NASK) src/asm/ipl.nas ipl.bin

tos.bin: src/asm/tos.nas Makefile
	$(NASK) src/asm/tos.nas tos.bin

%.gas : src/%.c Makefile
	$(CC1) -o $*.gas src/$*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj

utilfunc.obj: src/asm/utilfunc.nas Makefile 
	$(NASK) src/asm/utilfunc.nas utilfunc.obj

hankaku.bin : res/hankaku.txt Makefile
	$(MAKEFONT) res/hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(objects) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(objects)

bootpack.hrb : bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

tos.sys: tos.bin  bootpack.hrb Makefile
	copy /B tos.bin+bootpack.hrb tos.sys

tos.img: ipl.bin tos.sys Makefile
	$(EDIMG) imgin:$(TOOLPATH)fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:tos.sys to:@: \
		imgout:tos.img

img:
	$(MAKE) tos.img
	
asm:
	$(MAKE) ipl.bin

run:
	$(MAKE) img 
	$(COPY) tos.img d:\z_tools\qemu\fdimage0.bin
	$(TOOLPATH)make.exe -C $(TOOLPATH)/qemu

install:
	$(TOOLPATH)/make.exe img
	$(TOOLPATH)/imgtol.com w a:tos.img


clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) tos.sys
	-$(DEL) tos.img