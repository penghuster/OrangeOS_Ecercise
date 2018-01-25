BaseOfLoader equ 09000h ;;loader.bin被加载的位置
OffsetOfLoader equ 0100h ;;loader.bin 加载的偏移

BaseOfLoaderPhyAddr equ BaseOfLoader*10h ;;loader.bin被加载后的物理地址 <<4

BaseOfKernelFile equ 08000h ;Kernel.bin 被加载到的位置
OffsetOfKernelFile equ 0h  ;;Kernel.bin 被加载的偏移

BaseOfKernelFilePhyAddr equ BaseOfKernelFile * 10h 
KernelEntryPointPhyAddr equ 030400h ;;必须与 MAKEFILE中参数 -Ttext 的值相等

;PageDirBase equ 200000h  ;;页表开始地址 2M

