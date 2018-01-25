; 编译链接方法
; $ nasm -f elf kernel.asm -o kernel.o
; $ ld -s kernel.o -o kernel.bin    #‘-s’选项意为“strip all”

SELECTOR_KERNEL_CS equ 8

;;导入函数
extern cstart  

;;导入全局变量
extern gdt_ptr

[SECTION .bss]
StackSpace resb 2 * 1024
StackTop:

[section .text]	; 代码在此

global _start	; 导出 _start

_start:	; 跳到这里来的时候，我们假设 gs 指向显存

    ;;把esp从Loader挪到kernel
    mov esp, StackTop ;;堆栈再bss段中

    sgdt [gdt_ptr]  ;;cstart() 中将会用到gdt_ptr
    call cstart  ;;在此函数中改变了gdt_ptr，让他指向新的 GDT 
    lgdt [gdt_ptr]  ;;使用新的gdt 

    jmp  SELECTOR_KERNEL_CS:csinit  ;;
csinit:
    
    push 0
    popfd ;;Pop top of stack into EFLAGS

;    hlt

;.test:
	mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	mov	al, 'R'
	mov	[gs:((80 * 2 + 39) * 2)], ax	; 屏幕第 1 行, 第 39 列
	jmp	$
