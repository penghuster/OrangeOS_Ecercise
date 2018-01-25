org 07c00h  ;告诉编译器程序加载到 7c00处
mov ax, cs
mov ds, ax
mov es, ax
call DispStr   ;调用显示字符串例程
jmp $    ;无限循环

DispStr:
mov ax, BootMessage
mov bp, ax ;ES:BP = 串地址
mov cx, 16 ;CX=串长度
mov ax, 01301h  ;AH= 13h AL = 01h
mov bx, 000ch ;页号为0 黑底红字 0Ch
mov dl, 0
int 10h ; 10h中断号
ret 

BootMessage: db "Hello, OS world"
times 510-($-$$) db 0 ;填充剩下的空间，使生成的二进制代码恰好为512字节
dw 0xaa55 ;结束标志

