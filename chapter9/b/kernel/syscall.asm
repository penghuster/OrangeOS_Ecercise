%include "sconst.inc"

Nget_ticks equ 0  ;;要跟global.c 中sys_call_table定义相对应
NRprintx     equ 1
NRsendrec   equ 2

INT_VECTOR_SYS_CALL equ 0x90

;;global get_ticks ;;到处符号
global printx
global sendrec

bits 32
[section .text]

get_ticks:
    mov eax, Nget_ticks
    int INT_VECTOR_SYS_CALL
    ret


printx:
    mov eax, NRprintx
    mov ebx, [esp + 4]
    ;;mov ecx, [esp + 8]
    int INT_VECTOR_SYS_CALL
    ret

sendrec:
    mov eax, NRsendrec
    mov ebx, [esp + 4]  ;;function
    mov ecx, [esp + 8]  ;;src_dest
    mov edx, [esp + 12]  ;;p_msg
    int INT_VECTOR_SYS_CALL
    ret 


