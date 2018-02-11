
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                              klib.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                       Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

;[SECTION .data]
;disp_pos	dd	0

extern disp_pos

[SECTION .text]

; 导出函数
global	disp_str
global  out_byte
global  in_byte
global  disp_color_str
global enable_irq
global disable_irq



;;void disable_irq(int irq);
;;Disable an interrupt request line by setting an i8259 bit 
;;Equivalent code :
;;if(irq < 8)
;;out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq))
;;else
;;out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq))
disable_irq:
    mov ecx, [esp + 4]  ;;irq 
    pushf
    cli 

    mov ah, 1
    rol ah, cl ;; ah = (1 << (irq % 8))
    cmp cl, 8
    jae disable_8  ;;disable_irq >= 8 at the slave 8259
disable_0:
    in al, INT_M_CTLMASK
    test al, ah 
    jnz dis_already  ;; already disabled 
    or al, ah 
    out INT_M_CTLMASK, al  ;;set bit at master 8259
    popf 
    mov eax, 1 ;;disabled by this function 
    ret 
disable_8:  ;; for slave 8259
    in al, INT_S_CTLMASK
    test al, ah 
    jnz dis_already  ;; already disabled 
    or al, ah 
    out INT_S_CTLMASK, al  ;; set bit at slave  8259
    popf 
    mov eax, 1  ;;;disabled by this function , return value 
    ret 
dis_already:
    popf 
    xor eax, eax ;; already disabled  return value
    ret 

;;void enable_irq(int irq);
;;Enable an interrupt request line by clearing an 8259 int 
;;Equivalent code:
;;if(irq < 8)
;;out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq))
;;else
;;out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq))
enable_irq:
    mov ecx, [esp + 4]  ;; irq 
    pushf
    cli 

    mov ah, ~1
    rol ah, cl  ;;ah = ~(1 << (irq % 8))
    cmp cl, 8
    jae enable_8  ;;enable irq >=8 at the slave 8259
enable_0:
    in  al, INT_M_CTLMASK
    and al, ah 
    out INT_M_CTLMASK, al ;;clear bit at master 8259
    popf 
    ret 
enable_8:
    in al, INT_S_CTLMASK
    and al, ah 
    out INT_S_CTLMASK, al 
    popf
    ret 


;;;void out_byte(u16 port, u8 value)
out_byte:
    mov edx, [esp + 4] ; port
    mov al, [esp + 4 + 4] ;; value
    out dx, al 
    nop 
    nop 
    ret 

;;;u8 int_byte(u16 port)
in_byte:
    mov edx, [esp + 4]   ;;port
    xor eax, eax 
    in al, dx 
    nop 
    nop
    ret 

; ========================================================================
;                  void disp_str(char * info);
; ========================================================================
disp_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

disp_color_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, [ebp + 12] ;;color
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

