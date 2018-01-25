; ===========================================================
; pmtest1.asm
; 编译方法：nasm pmtest1.asm -o pmtest1.bin
; ==========================================================

%include "pm.inc"  ;定义一些常量，宏，以及一些说明

PageDirBase0 equ 200000h
PageTblBase0 equ 201000h ;;页表开始地址：2M + 4K
PageDirBase1 equ 210000h
PageTblBase1 equ 211000h

LinearAddrDemo equ 00401000h
ProcFoo  equ 00401000h
ProcBar  equ 00501000h
ProcPagingDemo equ 00301000h

org 0100h
    jmp LABEL_BEGIN

[SECTION .gdt]
; GDT
;段基址，段界限，属性
LABEL_GDT: Descriptor 0, 0, 0 ;空描述符
LABEL_DESC_NORMAL: Descriptor 0, 0ffffh, DA_DRW ;normal 描述符
LABEL_DESC_CODE32: Descriptor 0, SegCode32Len - 1, DA_CR + DA_32  ;非一致代码段
LABEL_DESC_CODE16: Descriptor 0, 0ffffh, DA_C; 非一致代码段  16
LABEL_DESC_DATA: Descriptor 0, DataLen-1, DA_DRW ;data 
LABEL_DESC_STACK: Descriptor 0, TopOfStack, DA_DRWA+DA_32 ;Stack 32位
;LABEL_DESC_TEST: Descriptor 0500000h, 0ffffh, DA_DRW 
LABEL_DESC_VIDEO: Descriptor 0B8000h, 0ffffh, DA_DRW ;显存首地址
;LABEL_DESC_PAGE_DIR: Descriptor PageDirBase, 4096  - 1, DA_DRW ;
;LABEL_DESC_PAGE_TBL: Descriptor PageTblBase, 4096*8 - 1, DA_DRW ;
LABEL_DESC_FLAT_C:  Descriptor 0, 0fffffh, DA_CR|DA_32|DA_LIMIT_4K
LABEL_DESC_FLAT_RW: Descriptor 0, 0fffffh, DA_DRW|DA_LIMIT_4K
; GDT 结束


GdtLen equ $ - LABEL_GDT  ;GDT 的长度
GdtPtr dw GdtLen - 1 ;GDT界限
dd 0 ; GDT基地址

;GDT选择子
SelectorNormal equ LABEL_DESC_NORMAL - LABEL_GDT
SelectorCode32 equ LABEL_DESC_CODE32 - LABEL_GDT
SelectorCode16 equ LABEL_DESC_CODE16 - LABEL_GDT
SelectorData equ LABEL_DESC_DATA - LABEL_GDT
SelectorStack equ LABEL_DESC_STACK - LABEL_GDT
;SelectorTest equ LABEL_DESC_TEST - LABEL_GDT
SelectorVideo equ LABEL_DESC_VIDEO - LABEL_GDT
;SelectorPageDir equ LABEL_DESC_PAGE_DIR - LABEL_GDT
;SelectorPageTbl equ LABEL_DESC_PAGE_TBL - LABEL_GDT
SelectorFlatC equ LABEL_DESC_FLAT_C - LABEL_GDT
SelectorFlatRW equ LABEL_DESC_FLAT_RW - LABEL_GDT
;END of [SECTION .gdt]

[SECTION .data1] ;数据段
ALIGN 32
[BITS 32]
LABEL_DATA:
;;实模式下使用这些符号
;;字符串
_szPMMessage:   db "In Protect Mode now. ^-^", 0ah, 0ah, 0 ;进入保护模式后显示此字符串
_szMemChkTitle: db "BaseAddrL BaseAddrH LengthLow LengthHigh     Type", 0ah, 0 
_szRAMSize      db "RAM size:", 0
_szReturn       db 0Ah, 0
_szHi           db "hello", 0ah, 0

;;变量
_wSPValueInRealMode     dw 0
_dwMCRNumber:       dd  0 ;;Memery check result
_dwDispPos:         dd  (80 * 6 + 0)* 2
_dwMemSize:         dd  0
_ARDStruct:   ;;Address Range Descriptor Structure
    _dwBaseAddrLow:     dd      0
    _dwBaseAddrHigh:    dd      0
    _dwLengthLow:       dd      0
    _dwLengthHigh:      dd      0
    _dwType:            dd      0
_PageTableNumber        dd      0
_SavedIDTR:  dd 0  ;;用于保存IDTR
             dd 0
_SavedIMREG:    dd  0    ;;;中断屏蔽寄存器

_MemChkBuf: times 256 db 0

szPMMessage equ _szPMMessage - $$
szMemChkTitle equ _szMemChkTitle - $$
szRAMSize   equ  _szRAMSize -$$
szReturn    equ  _szReturn - $$
szHi        equ _szHi - $$
dwDispPos   equ  _dwDispPos - $$
dwMemSize   equ  _dwMemSize - $$
dwMCRNumber equ  _dwMCRNumber - $$
ARDStruct   equ  _ARDStruct - $$
    dwBaseAddrLow equ _dwBaseAddrLow - $$
    dwBaseAddrHigh equ _dwBaseAddrHigh - $$
    dwLengthLow     equ _dwLengthLow - $$
    dwLengthHigh    equ _dwLengthHigh - $$
    dwType          equ _dwType - $$
PageTableNumber     equ _PageTableNumber - $$
SavedIDTR           equ _SavedIDTR - $$
SavedIMREG          equ _SavedIMREG - $$

MemChkBuf   equ _MemChkBuf - $$

DataLen equ $ - LABEL_DATA 
; END of [Section .data1]

;;IDT 
[SECTION .idt]
ALIGN 32
[BITS 32]
LABEL_IDT:
%rep 32
    Gate  SelectorCode32, SpuriousHandler, 0, DA_386IGate
%endrep
.020h:
    Gate SelectorCode32, ClockHandler, 0, DA_386IGate
%rep 95
    Gate SelectorCode32, SpuriousHandler, 0, DA_386IGate
%endrep 
.080h:
    Gate SelectorCode32, UserIntHandler, 0, DA_386IGate

IdtLen      equ $ - LABEL_IDT
IdtPtr      dw IdtLen - 1 ;;;段界限
            dd 0 ;;基地址
;[End  of SECTION .idt]


;全局堆栈段
[SECTION .gs]
ALIGN 32
[BITS 32]
LABEL_STACK:
times 512 db 0

TopOfStack equ $ - LABEL_STACK - 1
;end of [SECTION .gs]



[SECTION .s16]
[BITS 16]
LABEL_BEGIN:
    mov ax, cs 
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0100h

    mov [LABEL_GO_BACK_TO_REAL + 3], ax 
    mov [_wSPValueInRealMode], sp 

    ;;;利用 bios中断 int 15 得到内存描述信息
    mov ebx, 0  ;;;放置后续值，第一调用int 15 时必须为0
    mov di, _MemChkBuf  ;;es:di 指向一个地址范围描述符结构，BIOS将会填充此结构
.loop:
    mov eax, 0E820h ;;int 15 获取内存的功能号
    mov ecx, 20 ;;;bios填充字节数
    mov edx, 0534D4150h  ;;BIOS利用此标志，对调用者将要请求的系统映像信息进行校验。
    int 15h 
    jc LABEL_MEM_CHK_FAIL
    add di, 20   ;;;若是成功，则+20字节，移动到下一个地址范围描述符结构体
    inc dword [_dwMCRNumber]  ;;;地址范围描述符的计数+1
    cmp ebx, 0    ;;;若是ebx 为 0,则表示为最后一个地址范围描述符结构
    jne .loop 
    jmp LABEL_MEM_CHK_OK

LABEL_MEM_CHK_FAIL:
    mov dword [_dwMCRNumber], 0
LABEL_MEM_CHK_OK:



    ;;初始化 16位段描述符 
    mov ax, cs 
    movzx eax, ax 
    shl eax, 4
    add eax, LABEL_SEG_CODE16
    mov word [LABEL_DESC_CODE16 + 2], ax 
    shr eax, 16
    mov byte [LABEL_DESC_CODE16 + 4], al 
    mov byte [LABEL_DESC_CODE16 + 7], ah 

    ;;初始化32位代码段描述符
    xor eax, eax
    mov ax, cs
    shl eax, 4  ;此时处于实模式运行 其段基址与偏移的计算方式位 cs<<4 + offset
    add eax, LABEL_SEG_CODE32
    mov word [LABEL_DESC_CODE32 + 2], ax
    shr eax, 16
    mov byte [LABEL_DESC_CODE32 + 4], al
    mov byte [LABEL_DESC_CODE32 + 7], ah

    ;;初始化数据段描述符
    xor eax, eax 
    mov ax, ds 
    shl eax, 4
    add eax, LABEL_DATA 
    mov word [LABEL_DESC_DATA + 2], ax 
    shr eax, 16
    mov byte [LABEL_DESC_DATA + 4], al 
    mov byte [LABEL_DESC_DATA + 7], ah 

    ;;初始化堆栈描述符 
    xor eax, eax 
    mov ax, ds 
    shl eax, 4 
    add eax, LABEL_STACK 
    mov word [LABEL_DESC_STACK + 2], ax 
    shr eax, 16
    mov byte [LABEL_DESC_STACK + 4], al 
    mov byte [LABEL_DESC_STACK + 7], ah 


    ;;为加载GDTR做准备
    xor eax, eax
    mov ax, ds  
    shl eax, 4
    add eax, LABEL_GDT  ;eax <-gdt基地址
    mov dword [GdtPtr + 2], eax ; 将地址赋给GdtPtr高2个字节

    xor eax, eax
    mov ax, ds 
    shl eax, 4
    add eax, LABEL_IDT
    mov dword [IdtPtr + 2], eax

    sidt [_SavedIDTR]

    in al, 21h  ;;保存中断屏蔽寄存器的值o
    mov [_SavedIMREG], al

    
    ;;加载GDTR
    lgdt [GdtPtr]


    ;;关闭中断
    ;;cli
    lidt [IdtPtr]

    ;;打开地址线 A20
    in al, 92h
    or al, 00000010b 
    out 92h, al

    ;;准备切换到保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ;;真正进入保护模式
    jmp dword SelectorCode32:0  ;;执行此句会将Selectorcode32装入cs，
    ;;并跳转到Code32Selector:0处，

LABEL_REAL_ENTRY:
    mov ax, cs 
    mov ds, ax 
    mov es, ax 
    mov ss, ax 

    mov sp, [_wSPValueInRealMode]

    in al, 92h 
    and al, 11111101b
    out 92h, al 

    sti 

    mov ax, 4c00h 
    int 21h  ;;回到dos


;;END of [SECTION .s16]

[SECTION .s32] ;;进入32位代码 由实模式跳入
[BITS 32]

LABEL_SEG_CODE32:
    ;;;call SetupPaging 

    mov ax, SelectorVideo
    mov gs, ax ;视频段选择子
    mov ax, SelectorData
    mov es, ax 
    mov ax, SelectorData
    mov ds, ax 
    mov ax, SelectorStack
    mov ss, ax 

    mov esp, TopOfStack 

    call Init8295A

    int 080h 
    sti 
    jmp $

    ;;;下面显示一个字符串
    push szPMMessage
    call DispStr 
    add esp, 4
    ;
    call DispReturn;


    push szMemChkTitle
    call DispStr
    add esp, 4

    ;call DispReturn

    ;push dword [dwMCRNumber]
    ;call DispInt
    ;add esp, 4
    ;push dword [dwMemSize]
    ;call DispInt
    ;add esp, 4


    call DispMemSize 
    call PagingDemo

    call SetRealmode8259A

    ;;到此停止，再次跳转回 实模式
    jmp SelectorCode16:0

;;内存拷贝 仿 memcpy
;;void *MemCpy(void * es:pDest, void* ds:pSrc, int iSize);
MemCpy:
    push ebp 
    mov ebp, esp 

    push esi 
    push edi 
    push ecx 

    mov edi, [ebp + 8] ;;Destination
    mov esi, [ebp + 12] ;;Source 
    mov ecx, [ebp + 16]  ;;counter
.1:
    cmp ecx, 0 ;;判断计数器
    jz .2

    ;;逐个字节移动
    mov al, [ds:esi]
    inc esi 

    mov byte [es:edi], al 
    inc edi 

    dec ecx  ;;计数器减一循环
    jmp .1 
.2
    mov eax, [esp + 8]  ;;返回值
    pop ecx 
    pop edi 
    pop esi 
    mov esp, ebp 
    pop ebp 

    ret 
    
Init8295A:
    mov al, 011h ;;对于 ICW1 bit4 必须为 1； 如需要ICW4 bit0 为 1
    out 020h, al ;;IO地址020h为主8259, ICW1
    call io_delay

    out 0A0h, al  ;;0a0h为从8059的ICW1
    call io_delay

    mov al, 020h ;;IRQ0 对应中断向量 0x20
    out 021h, al ;;主8059， ICW2
    call io_delay

    mov al, 028h  ;;IRQ8对应中断向量 0x28
    out 0A1h, al   ;;从8059  ICW2 
    call io_delay

    mov al, 004h ;;IR2 对应从8259 
    out 021h, al  ;;主8259，ICW3
    call io_delay

    mov al, 002h  ;;对应主8259的IR2 
    out 0A1h, al  ;;从8259， icw3
    call io_delay

    mov al, 001h 
    out 021h, al ;;主8259， icw4 
    call io_delay
    
    out 0a1h, al ;;;从8259， icw4 
    call io_delay

    mov al, 11111110b  ;;仅仅开启定时器中断
    ;mov al, 11111111b ;;屏蔽主8059中所有中断
    out 021h, al ;;主8259，OCW1
    call io_delay

    mov al, 11111111b ;;;屏蔽从8259,所有中断
    out 0a1h, al ;;从8259， OCW1 
    call io_delay

    ret 

;;SetRealmode8259A 
SetRealmode8259A:
    mov ax, SelectorData
    mov fs, ax 

    mov al, 017h 
    out 020h, al  ;;主8259， ICW1 
    call io_delay

    mov al, 008h  ;;IRQ0，对应中断为0x8 
    out 021h, al  ;;主8259 
    call io_delay

    mov al, 001h 
    out 021h, al ;;主8259，icw4 
    call io_delay

    mov al, [fs:SavedIMREG]
    out 021h, al 
    call io_delay

    ret 
;;;end of  Setrealmode8259a 

io_delay:
    nop 
    nop 
    nop 
    nop 
    ret 

_ClockHandler:
ClockHandler equ _ClockHandler - $$
    inc byte [gs:((80*0 + 70) * 2)]
    mov al, 20h 
    out 20h, al ;;发送 EOI
    iretd

_UserIntHandler:
UserIntHandler equ _UserIntHandler - $$
    mov ah, 0ch 
    mov al, 'I'
    mov [gs:((80*0 + 75) * 2)], ax 
    iretd 

_SpuriousHandler:
SpuriousHandler equ _SpuriousHandler - $$
    mov ah, 0ch 
    mov al, '!'
    mov [gs:((80*0 + 75) * 2)], ax 
    jmp $
    iretd

SetupPaging:
    xor edx, edx 
    mov eax, [dwMemSize]
    mov ebx, 400000h ;;400000h = 4M = 4096 * 1024，一个页表对应的内存大小
    div ebx 
    mov ecx, eax ;;此时 ecx 为页表的个数，也即 PDE应该的个数
    test edx, edx 
    jz .no_remainder
    inc ecx  ;;如果余数不为零，则增加一个页表
.no_remainder:
;    push ecx ;;暂存页表个数；；
    mov [PageTableNumber], ecx 

    ;;为简化处理，所有线性地址对应相等的物理地址，并且不考虑内存空洞

    ;;初始化也目录
    ;mov ax, SelectorPageDir ;
    mov ax, SelectorFlatRW
    mov es, ax 
    ;;mov ecx, 1024 ;共 1k 个表项  此时页表个数已经在上一段代码中计算出来存于ecx中
    xor edi, edi
    mov edi, PageDirBase0

    xor eax, eax 
    mov eax, PageTblBase0 | PG_P | PG_USU | PG_RWW
.1:
    stosd  ;将eax 拷贝到 es:edi中
    add eax, 4096  ;页表索引+1，由于第十二位不用作页表索引，故+1也就是+4096
    loop .1 ;;共循环ecx次

    ;;初始化页表
    mov eax, [PageTableNumber]
    mov ebx, 1024
    mul ebx 
    mov ecx, eax ;PTE个数=页表个数*1024
    mov edi, PageTblBase0 ;;此段首地址 PageTblBase0
    xor eax, eax 
    mov eax, PG_P | PG_USU | PG_RWW 
.2:
    stosd 
    add eax, 4096 ;同样物理页也是4K对齐的，故物理页的索引都是4K的整数倍，故低12位作为属性位，对于索引来说相当于0
    loop .2

    mov eax, PageDirBase0
    mov cr3, eax 
    mov eax, cr0 
    or eax, 80000000h
    mov cr0, eax 
    jmp short .3
.3:
    nop

    ret 
;;分页基址完毕

;;测试分页机制
PagingDemo:
    mov ax, cs 
    mov ds, ax 
    mov ax, SelectorFlatRW
    mov es, ax  
;call DispHi
;ret 

    push LenFoo
    push OffsetFoo
    push ProcFoo
    call MemCpy
    add esp, 12 
;call DispHi
 
; ret
    push LenBar 
    push OffsetBar 
    push ProcBar 
    call MemCpy 
    add esp, 12

    push LenPagingDemoAll
    push OffsetPagingDemoProc
    push ProcPagingDemo 
    call MemCpy 
    add esp, 12 

    mov ax, SelectorData 
    mov ds, ax
    mov es, ax 

    call SetupPaging

    call SelectorFlatC:ProcPagingDemo
    call PSwitch  ;;切换页目录，改变地址映射关系
    call SelectorFlatC : ProcPagingDemo
    
    ret 

;;切换页表
PSwitch:
    ;;初始化页目录
    mov ax, SelectorFlatRW
    mov es, ax 
    mov edi, PageDirBase1  ;;此段首地址为 PageDirBase1
    xor eax, eax 
    mov eax, PageTblBase1 | PG_P | PG_USU | PG_RWW 
    mov ecx, [PageTableNumber]
.1:
    stosd
    add eax, 4096
    loop .1

    ;;再初始化所有页表
    mov eax, [PageTableNumber] ;;此段段首地址为 PageTblBase1
    mov ebx, 1024
    mul ebx 
    mov ecx, eax ;;PTE个数 = 页表个数 * 1024
    mov edi, PageTblBase1  ;;此段首地址为 PageTblBase1 
    xor eax, eax 
    mov eax, PG_P | PG_USU | PG_RWW 
.2:
    stosd
    add eax, 4096  ;;每页指向 4K 的空间
    loop .2

    ;;在此假设内存是大于 8M 的
    mov eax, LinearAddrDemo
    shr eax, 22
    mov ebx, 4096 
    mul ebx 
    mov ecx, eax 
    mov eax, LinearAddrDemo
    shr eax, 12 
    and eax, 03FFh ;;11111111111b 
    mov ebx, 4
    mul ebx 
    add eax, ecx 
    add eax, PageTblBase1
    mov dword [es:eax], ProcBar | PG_P | PG_USU | PG_RWW 

    mov eax, PageDirBase1
    mov cr3, eax 
    jmp short .3
.3:
    nop
    ret 
;;---------------------------------

PagingDemoProc:
OffsetPagingDemoProc equ PagingDemoProc - $$
    mov eax, LinearAddrDemo
    call eax 
    retf 
LenPagingDemoAll equ $ - PagingDemoProc

foo:
OffsetFoo equ foo - $$
    mov ah, 0ch 
    mov al, 'F'
    mov [gs:((80 * 18 + 0) * 2)], ax 
    mov al, 'o'
    mov [gs:((80 * 18 + 1) * 2)], ax 
    mov [gs:((80 * 18 + 2) * 2)], ax 
    ret 
LenFoo equ $ - foo 

bar:
OffsetBar equ bar - $$
    mov ah, 0ch 
    mov al, 'B'
    mov [gs:((80 * 19 + 0) * 2)], ax 
    mov al, 'a'
    mov [gs:((80 * 19 + 1) * 2)], ax
    mov al, 'r'
    mov [gs:((80 * 19 + 2) * 2)], ax
    ret
LenBar      equ $ - bar 


DispMemSize:
    push esi
    push edi
    push ecx 

    mov esi, MemChkBuf
    mov ecx, [dwMCRNumber]
;    jmp .test
.loop:
    mov edx, 5
    mov edi, ARDStruct
.1:
    push dword [esi]
    call DispInt
    pop eax 
    stosd
    add esi, 4
    dec edx 
    cmp edx, 0
    jnz .1
    call DispReturn
    cmp dword [dwType], 1 ;;if(Type == AddressRangeMemory)
    jne .2 
    mov eax, [dwBaseAddrLow]
    add eax, [dwLengthLow]
    cmp eax, [dwMemSize]
    jb .2
    mov [dwMemSize], eax;
.2:
    loop .loop 
;.test:
;    call DispReturn
    push szRAMSize
    call DispStr
    add esp, 4

    push dword [dwMemSize]
    call DispInt
    add esp, 4 

    pop ecx 
    pop edi
    pop esi
    ret 

%include "lib.inc"

SegCode32Len equ $ - LABEL_SEG_CODE32
;;end of [SECTION .s32]


;;16位代码段，由32位跳入，从而进入实模式
[SECTION .s16code]
ALIGN 32 
[BITS 16]
LABEL_SEG_CODE16:
mov ax, SelectorNormal 
mov ds, ax 
mov es, ax 
mov fs, ax 
mov gs, ax 
mov ss, ax 

mov eax, cr0 
and eax, 7FFFFFFEh 
mov cr0, eax 

LABEL_GO_BACK_TO_REAL:
jmp 0:LABEL_REAL_ENTRY 

Code16Len equ $-LABEL_SEG_CODE16



