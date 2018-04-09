; 编译链接方法
; $ nasm -f elf kernel.asm -o kernel.o
; $ ld -s kernel.o -o kernel.bin    #‘-s’选项意为“strip all”

%include "sconst.inc"

;;SELECTOR_KERNEL_CS equ 8

;;导入函数
extern cstart  
extern exception_handler
extern spurious_irq
extern kernel_main
extern clock_handler


;;导入全局变量
extern gdt_ptr
extern idt_ptr
extern p_proc_ready
extern tss 
extern disp_str
extern delay
extern k_reenter
extern irq_table
extern sys_call_table


[SECTION .bss]
StackSpace resb 2 * 1024
StackTop:

[SECTION .data]
clock_int_msg   db "^", 0

[section .text]	; 代码在此

global _start	; 导出 _start
global restart
global sys_call 

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
    
;    sti 
;    hlt
    ;push 0
    ;popfd ;;Pop top of stack into EFLAGS
;    ud2
;    jmp 0x48:0
;    hlt

    xor eax, eax 
    mov ax, SELECTOR_TSS
    ltr ax

    jmp kernel_main

;.test:
	mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	mov	al, 'R'
	mov	[gs:((80 * 2 + 39) * 2)], ax	; 屏幕第 1 行, 第 39 列
	jmp	$


;;硬件中断
%macro hwint_master 1

    call save 

    in al, INT_M_CTLMASK
    or al, (1 << %1) ;;不允许再发生时钟中断
    out INT_M_CTLMASK, al 

    ;/通知i8259中断结束
    mov al, EOI  ;;reenable
    out INT_M_CTL, al  ;;master 8259

    sti

    push %1
    call [irq_table + 4 * %1]
    pop ecx

    cli 

    in al, INT_M_CTLMASK
    and al, ~(1 << %1)
    out INT_M_CTLMASK, al 

    ret  ;;若是重入则跳转到 .restart_reenter_v2  若是非重入则跳转到 .restart_v2 
%endmacro


save:
 ;   hwint_master  0
    ;;此时由于从用户进程跳转而来，故此时esp指向刚刚正在执行进程的
    ;;PCB, 中断发生cpu保存 cs eflags eip 寄存器到当前堆栈，sub esp 4
    ;;则是跳过 retaddr，继续push寄存器 pushad ds es fs gs寄存器
    pushad 
    push ds 
    push es 
    push fs 
    push gs 

;;此处由于进行了特权级转换，故ss已经加载为内核栈基址
;;注意,从这里开始一直到"mov esp, StackTop",中间坚决不能使用"pop/push"指令.
;;因为当前esp指向PCB中的某个位置,push/pop会破坏进程表,带来灾难性后果
    
    ;;保存edx, 因为edx里保存了系统调用的参数
    ;;此时esp指向pcb不能任意使用
    mov esi, edx
    
    mov dx, ss 
    mov ds, dx 
    mov es, dx 
    
    mov edx, esi ;;恢复edx

    mov esi, esp ;;eax = 进程起始地址

    inc dword [k_reenter]
    cmp dword [k_reenter], 0
    jne .1
    mov esp, StackTop
    push restart 
    jmp [esi + RETADR - P_STACKBASE]
.1:
    push restart_reenter
    jmp [esi + RETADR - P_STACKBASE]

ALIGN 16
hwint00:   ;;Iterrupt routine for irq 0(the clock)
    hwint_master 0

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
    call save 
    in al, INT_S_CTLMASK 
    or al, (1 >> (%1 - 8))  ;;屏蔽当前中断
    out INT_S_CTLMASK, al 

    ;;通知当前中断结束，注意主从中断芯片都需要通知
    mov al, EOI  ;;置EOI位
    out INT_M_CTL, al 
    nop 
    out INT_S_CTL, al ;;

    sti ;;CPU在响应中断的过程中会自动关中断，这句之后就允许响应新的中断

    push %1
    call [irq_table + 4 * %1]  ;;中断处理程序
    pop ecx 
    cli 

    in al, INT_S_CTLMASK
    and al, ~(1 << (%1 - 8))  ;;恢复接收当前中断
    out INT_S_CTLMASK, al 
    ret 
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
;;    iretd
;;    jmp $
    sti
    hlt 
    mov ah, 0Fh             ; 0000: 黑底    1111: 白字
    mov al, 'C'
    mov [gs:((80 * 2 + 39) * 2)], ax    ; 屏幕第 1 行, 第 39 列

    jmp $


sys_call:
    call save 
    sti 

    push esi ;;下面用到此寄存器
    push dword [p_proc_ready]
    push edx
    push ecx 
    push ebx
    call [sys_call_table + eax * 4]
    add esp, 4*4
    pop esi
    mov [esi + EAXREG - P_STACKBASE], eax 
    
    cli 
    ret 



restart:
    ;;加载下一个要执行进程的ldtr
    mov esp, [p_proc_ready]
    lldt [esp + P_LDT_SEL]

    ;;将PCB中 esp所在位置，赋值给tss中esp,待切换特权级后，esp指向此位置
    lea eax, [esp + P_STACKTOP]
    mov dword [tss + TSS3_S_SP0], eax
restart_reenter:
    dec dword [k_reenter]
    pop gs 
    pop fs 
    pop es 
    pop ds 
    popad 

    add esp, 4

    iretd
