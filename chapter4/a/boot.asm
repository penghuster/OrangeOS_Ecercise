org 07c00h  ;告诉编译器程序加载到 7c00处


jmp short LABEL_START 
nop ;;这个nop必不可少

;;下面是FAT12磁盘的头
BS_OEMName DB 'ForrestY'  ;;oem String 必须为8个字节
BPB_BytsPerSec  DW 512  ;;每个扇区的字节数
BPB_SecPerClus  DB 1  ;;每个族多少片
BPB_RsvdSecCnt  DW 1 ;;boot记录站多少个扇区
BPB_NumFATs     DB 2  ;;共有多少 FAT 表
BPB_RootEntCnt  DW 224 ;;根目录文件数最大值
BPB_TotSec16    DW  2880 ;;逻辑扇区总数
BPB_Media       DB  0xF0  ;;媒体描述符
BPB_FATSz16     DW  9   ;;每个FAT扇区数
BPB_SecPerTrk   DW 18 ;;每磁道扇区数
BPB_NumHeads    DW 2        ;;磁头数（面数）
BPB_Hiddsec     DD  0  ;;隐藏扇区数
BPB_TotSec32    DD 0 ;;wTotalSectorCount为0时这个值记录扇区数
BS_DrvNum       DB 0 ;;中断13的驱动器号
BS_Reserved1    DB 0 ;;未使用
BS_BootSig      DB 29h ;;扩展引导标记
BS_VolID        DD  0  ;;卷序列号 
BS_VolLab       DB 'OrangeS0.02'  ;;卷标，必须11个字节
BS_FileSysType  DB  'FAT12   '   ;;文件系统类型， 必须为8个字节

LABEL_START:
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

