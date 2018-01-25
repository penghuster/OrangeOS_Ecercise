; 编译链接方法
; $ nasm -f elf kernel.asm -o kernel.o
; $ ld -s kernel.o -o kernel.bin    #‘-s’选项意为“strip all”

SELECTOR_KERNEL_CS equ 8

;;导入函数
extern cstart  
extern exception_handler
extern spurious_irq

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

;;硬中断
global hwint00
global hwint01
global hwint02
global hwint03
global hwint04
global hwint05
global hwint06
global hwint07
global hwint08
global hwint09
global hwint10
global hwint11 
global hwint12
global hwint13
global hwint14
global hwint15





_start:	; 跳到这里来的时候，我们假设 gs 指向显存

    ;;把esp从Loader挪到kernel
    mov esp, StackTop ;;堆栈再bss段中

    sgdt [gdt_ptr]  ;;cstart() 中将会用到gdt_ptr
    call cstart  ;;在此函数中改变了gdt_ptr，让他指向新的 GDT 
    lgdt [gdt_ptr]  ;;使用新的gdt 

    lidt [idt_ptr]

    jmp  SELECTOR_KERNEL_CS:csinit  ;;
csinit:
    
    sti 
    hlt
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


;;硬件中断
%macro hwint_master 1
        push %1
        call spurious_irq
        add esp, 4
        sti
        hlt 
        ;jmp $
%endmacro

ALIGN 16
hwint00:   ;;Iterrupt routine for irq 0(the clock)
    hwint_master  0

ALIGN 16
hwint01:  ;/irq 1 (keyboard)
    hwint_master 1

ALIGN 16
hwint02:  ;/irq 3 (cascade!)
    hwint_master 2

ALIGN 16
hwint03:   ;;irq 3   (second serial)
    hwint_master 3

ALIGN 16
hwint04:    ;;irq 4 (first serial)
    hwint_master 4

ALIGN 16
hwint05:   ;;irq xt winchester
    hwint_master 5

ALIGN 16
hwint06:  ;; floppy
    hwint_master 6 

ALIGN 16
hwint07:   ;;printer
    hwint_master 7  

;;;--------------------
%macro hwint_slave 1
    push %1
    call spurious_irq
    add esp, 4
    sti
    hlt
    ;jmp $
%endmacro
;;;------------------

ALIGN 16
hwint08:   ;;realtime clock
    hwint_slave 8

ALIGN 16
hwint09:    ;;redirected
    hwint_slave 9

ALIGN 16
hwint10:
    hwint_slave 10

ALIGN 16
hwint11:
    hwint_slave 11

ALIGN 16
hwint12:
    hwint_slave 12

ALIGN 16
hwint13:
    hwint_slave 13

ALIGN 16
hwint14:
    hwint_slave 14

ALIGN 16
hwint15:
    hwint_slave 15



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

