; ===========================================================
; pmtest1.asm
; 编译方法：nasm pmtest1.asm -o pmtest1.bin
; ==========================================================

%include "pm.inc"  ;定义一些常量，宏，以及一些说明

org 0100h
jmp LABEL_BEGIN

[SECTION .gdt]
; GDT
;段基址，段界限，属性
LABEL_GDT: Descriptor 0, 0, 0 ;空描述符
LABEL_DESC_NORMAL: Descriptor 0, 0ffffh, DA_DRW ;normal 描述符
LABEL_DESC_CODE32: Descriptor 0, SegCode32Len - 1, DA_C + DA_32  ;非一致代码段
LABEL_DESC_CODE16: Descriptor 0, 0ffffh, DA_C; 非一致代码段  16
LABEL_DESC_DATA: Descriptor 0, DataLen-1, DA_DRW ;data 
LABEL_DESC_STACK: Descriptor 0, TopOfStack, DA_DRWA+DA_32 ;Stack 32位
LABEL_DESC_TEST: Descriptor 0500000h, 0ffffh, DA_DRW 
LABEL_DESC_VIDEO: Descriptor 0B8000h, 0ffffh, DA_DRW ;显存首地址
LABEL_DESC_LDT: Descriptor 0, LDTLen - 1, DA_LDT  ;;LDT
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
SelectorTest equ LABEL_DESC_TEST - LABEL_GDT
SelectorVideo equ LABEL_DESC_VIDEO - LABEL_GDT
SelectorLDT equ LABEL_DESC_LDT - LABEL_GDT
;END of [SECTION .gdt]

[SECTION .data1] ;数据段
ALIGN 32
[BITS 32]
LABEL_DATA:
SPValueInRealMode dw 0
;字符串
PMMessage: db "In.Protect.Mode.now. ^-^", 0   ;在保护模式中显示
OffsetPMMessage equ PMMessage - $$
strTest: db "ABCDEFGGIJKLMNOPQRSTUVWXYZ", 0
OffsetStrTest equ strTest - $$
DataLen equ $ - LABEL_DATA 
; END of [Section .data1]

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
mov [SPValueInRealMode], sp 

;;初始化15位段描述符 
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

;;初始化GDT中 LDT的描述符
xor eax, eax 
mov ax, ds 
shl eax, 4
add eax, LABEL_LDT 
mov word [LABEL_DESC_LDT + 2], ax 
shr eax, 16
mov byte [LABEL_DESC_LDT + 4], al 
mov byte [LABEL_DESC_LDT + 7], ah

;初始化 LDT中代码段 .la 的描述符
xor eax, eax 
mov ax, ds 
shl eax, 4
add eax, LABEL_CODE_A
mov word [LABEL_LDT_DESC_CODEA + 2], ax 
shr eax, 16
mov byte [LABEL_LDT_DESC_CODEA + 4], al 
mov byte [LABEL_LDT_DESC_CODEA + 7], ah 

xor eax, eax 
mov ax, ds 
shl eax, 4
add eax, LABEL_DATA2 
mov word [LABEL_LDT_DESC_DATA2 + 2], ax 
shr eax, 16 
mov byte [LABEL_LDT_DESC_DATA2 + 4], al 
mov byte [LABEL_LDT_DESC_DATA2 + 7], ah 

;;为加载GDTR做准备
xor eax, eax
mov ax, ds  
shl eax, 4
add eax, LABEL_GDT  ;eax <-gdt基地址
mov dword [GdtPtr + 2], eax ; 将地址赋给GdtPtr高2个字节


;;加载GDTR
lgdt [GdtPtr]


;;关闭中断
cli

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

mov sp, [SPValueInRealMode]

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
mov ax, SelectorVideo
mov gs, ax ;视频段选择子
mov ax, SelectorTest
mov es, ax 
mov ax, SelectorData
mov ds, ax 
mov ax, SelectorStack
mov ss, ax 

mov esp, TopOfStack 

;下面显示一个字符
mov ah, 0ch ;; 0000 黑底， 1100：红字
xor esi, esi
xor edi, edi 
mov esi, OffsetPMMessage ;;源数据偏移
mov edi, (80*10 + 0) * 2
cld  ;;//清除方向标识，使其为 0
.1:
lodsb  ;;//将ds中si所指向的内容按字节加载到al中，由于方向标志为0，故si自动+1
test al, al 
jz .2
mov [gs:edi], ax 
add edi, 2
jmp .1
.2:  ;显示完毕

call DispReturn

mov ax, SelectorLDT 
lldt ax 

jmp SelectorLDTCodeA:0

call TestRead
call TestWrite
call TestRead


;;到此停止，再次跳转回 实模式
jmp SelectorCode16:0

TestRead:
xor esi, esi
mov ecx, 8
.loop:
mov al, [es:esi]
call DispAL
inc esi
loop .loop

call DispReturn

ret
;;Testread 结束

TestWrite:
push esi 
push edi
xor esi, esi
xor edi, edi
mov esi, OffsetStrTest ;;源数据
cld 
.1:
lodsb
test al, al 
jz .2
mov [es:edi], al 
inc edi
jmp .1
.2:

pop edi
pop esi 

ret
;;Testwrite 结束

;;
;;显示AL中的数字
;;默认地：
;;数字已经存在AL中
;;edi始终指向要显示的下一个字符的位置
;;被改变的寄存器 ax, edi 
DispAL:
push edx
push ecx

mov ah, 0ch 
mov dl, al 
shr al, 4
mov ecx, 2
.begin:
and al, 01111b 
cmp al, 9
ja .1
add al, '0'
jmp .2
.1:
sub al, 0Ah
add al, 'A'
.2:
mov [gs:edi], ax 
add edi, 2
mov al, dl 
loop .begin
add edi, 2

pop edx 
pop ecx
ret
;;DispAL 结束

DispReturn:
push eax 
push ebx 
mov eax, edi
mov bl, 160
div bl 
and eax, 0ffh 
inc eax 
mov bl, 160
mul bl 
mov edi, eax 
pop ebx 
pop eax 

ret
;;Dispreturn 结束




;mov edi, (80 * 11 + 79) * 2 
;mov ah, 0ch 
;mov al, 'D'
;mov [gs:edi], ax

;;end
jmp $

SegCode32Len equ $ - LABEL_SEG_CODE32
;;end of [SECTION .s32]

[SECTION .ldt]
ALIGN 32 
LABEL_LDT:

;;LDT 段基址 段界限 属性
LABEL_LDT_DESC_CODEA: Descriptor 0, CodeALen - 1, DA_C + DA_32; + SA_TIL; code 
LABEL_LDT_DESC_DATA2: Descriptor 0, LdtDataLen - 1, DA_DRW + DA_32 

LDTLen equ $ - LABEL_LDT;

;LDT 选择子
SelectorLDTCodeA equ LABEL_LDT_DESC_CODEA - LABEL_LDT + SA_TIL
SelectorLDTData2 equ LABEL_LDT_DESC_DATA2 - LABEL_LDT + SA_TIL
;end of [SECTION .ldt]

[SECTION .data2]
ALIGN 32
[BITS 32]
LABEL_DATA2:
;字符串
LDTMessage: db "LDT Data Test.", 0
OffsetLDTMessage equ LDTMessage - $$

LdtDataLen equ $ - LABEL_DATA2
;end of [SECTION .data2]

;CodeA (LDT, 32位代码段)
[SECTION .la]
ALIGN 32 
[BITS 32]
LABEL_CODE_A:
mov ax, SelectorVideo
mov gs, ax ;视屏段选择子
;mov edi, (80 * 12 + 0) * 2 ;屏幕第 10 行 第0列
;;mov ah, 0ch ;0000黑底， 1100：红字
;;mov al, 'L'
;;mov [gs:edi], ax;
mov ax, SelectorLDTData2
mov ds, ax
mov ah, 0ch 
xor esi, esi
xor edi,edi
mov esi, OffsetLDTMessage
mov edi, (80*14 + 0) * 2
cld
.1:
lodsb
test al, al 
jz .2
mov [gs:edi], ax 
add edi, 2 
jmp .1
.2:

;;准备经由16位代码段跳回实模式
jmp SelectorCode16:0
CodeALen equ $ - LABEL_CODE_A
;;end of [SECTION .la]

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
and al, 11111110b 
mov cr0, eax 

LABEL_GO_BACK_TO_REAL:
jmp 0:LABEL_REAL_ENTRY 

Code16Len equ $-LABEL_SEG_CODE16
;;end of [SECTION .s16code]


