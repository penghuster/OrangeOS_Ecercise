org 0100h

mov ax, 0b800h
mov gs, ax
mov ah, 0Fh  ;;0000黑底  1111 白字
mov al, 'L'
mov [gs:(80 * 0 + 39) * 2], ax 

jmp $ ;;到此停住


