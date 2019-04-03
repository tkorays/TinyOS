; TAB=4

BOTPAK	EQU		0x00280000
DSKCAC	EQU		0x00100000
DSKCAC0	EQU		0x00008000

CYLS    EQU 0x0ff0
LEDS    EQU 0x0ff1
VMODE   EQU 0x0ff2
SCRNX   EQU 0x0ff4
SCRNY   EQU 0x0ff6
VRAM    EQU 0x0ff8

    ORG     0xc200


    MOV     AL, 0x13
    MOV     AH, 0x00
    INT     0x10

    MOV     BYTE [VMODE], 8
    MOV     WORD [SCRNX], 320
    MOV     WORD [SCRNY], 200
    MOV     DWORD [VRAM], 0x000a0000

    MOV     AH, 0x02
    INT     0x16        ; 键盘BIOS
    MOV     [LEDS], AL

; 禁止所有中断
    MOV     AL, 0xff
    OUT     0x21, AL
    NOP
    OUT     0xa1, AL

    CLI

; 设置A20GATE，激活电路，访问1M以上内存
    CALL    waitkbdout
    MOV     AL, 0xd1
    OUT     0x64, AL
    CALL    waitkbdout
    MOV     AL, 0xdf
    OUT     0x60, AL 
    CALL    waitkbdout

; EAX LGDT等命令为i386之后的指令
[INSTRSET "i486p"]
    LGDT    [GDTR0]
    MOV     EAX, CR0
    AND     EAX, 0x7fffffff     ;设置bit31为0，禁止分页
    OR      EAX, 0x00000001     ;设置bit0位1，切换为保护模式
    MOV     CR0, EAX 
    JMP     pipelineflush

;通过带入CR0切换保护模式时，需要马上执行JMP
pipelineflush:
    MOV     AX, 1*8
    MOV     DS, AX              ; 段寄存器设置为0x08
    MOV     ES, AX
    MOV     FS, AX
    MOV     GS, AX 
    MOV     SS, AX

; bootpack，文件最后的一个标签
    MOV     ESI, bootpack
    MOV     EDI, BOTPAK 
    MOV     ECX, 512*1024/4     ; 复制512kB，保证能够容纳下我们的c代码
    CALL    memcpy

; 将启动扇区的512字节复制到DSKCAC(0x00100000)，即1M以后的地方
    MOV     ESI, 0x7c00
    MOV     EDI, DSKCAC
    MOV     ECX, 512/4
    CALL    memcpy

; 将0x00008200的内容复制到0x00100200地方
    MOV     ESI, DSKCAC0+512
    MOV     EDI, DSKCAC+512
    MOV     ECX, 0
    MOV     CL, BYTE [CYLS]
    IMUL    ECX, 512*18*2/4
    SUB     ECX, 512/4
    CALL    memcpy 


    MOV     EBX, BOTPAK
    MOV     ECX, [EBX+16]
    ADD     ECX, 3
    SHR     ECX, 2
    JZ      skip
    MOV     ESI, [EBX+20]
    ADD     ESI, EBX
    MOV     EDI, [EBX+12]
    CALL    memcpy
skip:
    MOV     ESP, [EBX+12]
    JMP     DWORD 2*8:0x0000001b

waitkbdout:
    IN      AL, 0x64
    AND     AL, 0x02
    ;IN      AL, 0x60
    JNZ     waitkbdout
    RET

memcpy:
    MOV     EAX, [ESI]
    ADD     ESI, 4
    MOV     [EDI], EAX 
    ADD     EDI, 4
    SUB     ECX, 1
    JNZ     memcpy
    RET

; 如果GDT0标签不是8的倍数访问速度会慢，这里16有点多
    ALIGNB  16

GDT0:
    RESB    8
    DW      0xffff,0x0000,0x9200,0x00cf  
    DW      0xffff,0x0000,0x9a28,0x0047
    DW      0

GDTR0:
    DW      8*3-1
    DD      GDT0 

    ALIGNB  16

; c语言生成的二进制紧接着后面
bootpack:
