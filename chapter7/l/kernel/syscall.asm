%include "sconst.inc"

Nget_ticks equ 0  ;;要跟global.c 中sys_call_table定义相对应
INT_VECTOR_SYS_CALL equ 0x90

global get_ticks ;;到处符号

bits 32
[section .text]

get_ticks:
    mov eax, Nget_ticks
    int INT_VECTOR_SYS_CALL
    ret
