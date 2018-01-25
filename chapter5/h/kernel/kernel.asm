; 编译链接方法
; $ nasm -f elf kernel.asm -o kernel.o
; $ ld -s kernel.o -o kernel.bin    #‘-s’选项意为“strip all”

SELECTOR_KERNEL_CS equ 8

;;导入函数
extern cstart  
extern exception_handler

;;导入全局变量
extern gdt_ptr
extern idt_ptr

[SECTION .bss]
StackSpace resb 2 * 1024
StackTop:

[section .text]	; 代码在此

global _start	; 导出 _start

global divide_error
global single_step_exception
global nmi
global breakpoint_exception
global overflow
global bounds_check
global inval_opcode 
global copr_not_available
global double_fault 
global copr_seg_overrun
global inval_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global copr_error 


_start:	; 跳到这里来的时候，我们假设 gs 指向显存

    ;;把esp从Loader挪到kernel
    mov esp, StackTop ;;堆栈再bss段中

    sgdt [gdt_ptr]  ;;cstart() 中将会用到gdt_ptr
    call cstart  ;;在此函数中改变了gdt_ptr，让他指向新的 GDT 
    lgdt [gdt_ptr]  ;;使用新的gdt 

    lidt [idt_ptr]

    jmp  SELECTOR_KERNEL_CS:csinit  ;;
csinit:
    
    ;push 0
    ;popfd ;;Pop top of stack into EFLAGS
;    ud2
    jmp 0x48:0
;    hlt

;.test:
	mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	mov	al, 'R'
	mov	[gs:((80 * 2 + 39) * 2)], ax	; 屏幕第 1 行, 第 39 列
	jmp	$

divide_error:
    push 0xFFFFFFFF ;no err code 
    push 0 ;;vector_no = 0
    jmp exception

single_step_exception:
    push 0xFFFFFFFF  ;;no err code 
    push 1 ;vector_no = 1
    jmp exception

nmi:
    push 0xFFFFFFFF  ;;no err code 
    push 2 ;;vector_no= 2
    jmp exception

breakpoint_exception:
    push 0xFFFFFFFF  ;;no err code 
    push 3  ;;vector_no = 3
    jmp exception

overflow:
    push 0xFFFFFFFF  ;; no err code 
    push 4 ;;vector_no = 4
    jmp exception

bounds_check:
    push 0xFFFFFFFF ;;no err code 
    push 5 ;;vector_no = 5
    jmp exception

inval_opcode:
    push 0xFFFFFFFF ;;no err code 
    push 6 ;;vector_no = 6
    jmp exception

copr_not_available:
    push 0xFFFFFFFF ;;no err code 
    push 7
    jmp exception

double_fault:
    push 8  ;;vector_no = 8
    jmp exception

copr_seg_overrun:
    push 0xFFFFFFFF ;;no err code 
    push 9 ;;vector_no = 9
    jmp exception

inval_tss:
    push 10 ;;vector_no = 10
    jmp exception

segment_not_present:
    push 11 ;;vector_no = 11
    jmp exception

stack_exception:
    push 12 ;;vector_no = C 
    jmp exception

general_protection:
    push 13 ;;vector_no = D 
    jmp exception

page_fault:
    push 14 ;;vector_no = E 
    jmp exception

copr_error:
    push 0xFFFFFFFF  ;; no err code 
    push 16 ;;vector_no = 10h 
    jmp exception


exception:
    call exception_handler
    add esp, 4*2
    jmp $
;    hlt 
    mov ah, 0Fh             ; 0000: 黑底    1111: 白字
    mov al, 'C'
    mov [gs:((80 * 2 + 39) * 2)], ax    ; 屏幕第 1 行, 第 39 列

    jmp $


