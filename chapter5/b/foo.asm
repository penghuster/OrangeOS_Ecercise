;编译链接方法 
;(ld 的 ‘-s’ 选项意为 “strip all”)
;
; $ nasm -f elf foo.asm -o foo.o 
; $ gcc -c bar.c -o bar.o 
; $ ld -s foo.o bar.o -o foobar
; $ ./foobar
; the 2nd one 
; $

extern choose; int choose(int a, int b);申明是一个外部函数

[section .data] ;;数据段

num1st dd 3
num2nd dd 4

[section .text] ;;代码段

global _start ;;必须到处 _start 这个入口，以便让编译器识别
global myprint ;;导出此函数是为了让 bar.c使用

_start:
    push dword [num2nd]
    push dword [num1st]
    call choose ;;choose(num1st, num2nd)
    add esp, 8

    mov ebx, 0
    mov eax, 1 ;;sys_exit
    int 0x80 ;;system call 

    ;;void myprint(char* msg, int len)
myprint:
    mov edx, [esp + 8] ;;len 
    mov ecx, [esp + 4] ;;msg

    mov ebx, 1 
    mov eax, 4 ;;sys_write 
    int 0x80 
    ret 
