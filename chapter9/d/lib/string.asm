
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                              string.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                       Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

[SECTION .text]

; 导出函数
global	memcpy
global memset
global strcpy
global strlen


; ------------------------------------------------------------------------
; void* memcpy(void* es:pDest, void* ds:pSrc, int iSize);
; ------------------------------------------------------------------------
memcpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回
; memcpy 结束-------------------------------------------------------------



;;void memset(void* p_dst, char ch, int size)
memset:
    push ebp 
    mov ebp, esp 

    push esi 
    push edi 
    push ecx 

    mov edi, [ebp + 8]
    mov edx, [ebp + 12]
    mov ecx, [ebp + 16]

.1:
    cmp ecx, 0
    jz .2

    mov byte [edi], dl
    inc edi 
    
    dec ecx 
    jmp .1

.2: 
    pop ecx 
    pop edi 
    pop esi 
    mov esp, ebp 
    pop ebp 

    ret


;;char * strcpy(char* p_dst, char* p_src);
strcpy:
    push ebp
    mov  ebp, esp

    mov esi, [ebp + 12]  ;;source
    mov edi, [ebp + 8]  ;;destination

.1:
    mov al, [esi]
    inc esi 

    mov byte [edi], al
    inc edi

    cmp al, 0  ;;判断是否遇到 ‘\0’
    jnz .1

    mov eax, [ebp + 8]  ;;返回值

    pop ebp 
    ret 
;;strcpy end 


strlen:
    push ebp 
    mov ebp, esp 

    mov eax, 0   ;;字符串长度开始为0
    mov esi, [ebp + 8]   ;;esi指向首地址

.1:
    cmp byte [esi], 0
    jz  .2
    inc esi 
    inc eax 
    jmp .1

.2:
    pop ebp 
    ret 
