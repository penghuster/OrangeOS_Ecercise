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


;;初始化32位代码段描述符
xor eax, eax
mov ax, cs
shl eax, 4  ;此时处于实模式运行 其段基址与偏移的计算方式位 cs<<4 + offset
add eax, LABEL_SEG_CODE32
mov word [LABEL_DESC_CODE32 + 2], ax
shr eax, 16
mov byte [LABEL_DESC_CODE32 + 4], al
mov byte [LABEL_DESC_CODE32 + 7], ah


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

;;END of [SECTION .s16]

[SECTION .s32] ;;进入32位代码 由实模式跳入
[BITS 32]

LABEL_SEG_CODE32:
mov ax, SelectorVideo
mov gs, ax ;视频段选择子

mov edi, (80 * 11 + 79) * 2 
mov ah, 0ch 
mov al, 'D'
mov [gs:edi], ax

;;end
jmp $

SegCode32Len equ $ - LABEL_SEG_CODE32
;;end of [SECTION .s32]

