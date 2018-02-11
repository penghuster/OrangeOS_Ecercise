%include "sconst.inc"

Nget_ticks equ 0  ;;要跟global.c 中sys_call_table定义相对应
NRwrite     equ 1

INT_VECTOR_SYS_CALL equ 0x90

global get_ticks ;;到处符号
global write

bits 32
[section .text]

get_ticks:
    mov eax, Nget_ticks
    int INT_VECTOR_SYS_CALL
    ret


write:
    mov eax, NRwrite
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    int INT_VECTOR_SYS_CALL
    ret
