BaseOfLoader equ 09000h ;;loader.bin被加载的位置
OffsetOfLoader equ 0100h ;;loader.bin 加载的偏移

BaseOfLoaderPhyAddr equ BaseOfLoader*10h ;;loader.bin被加载后的物理地址 <<4

BaseOfKernelFile equ 08000h ;Kernel.bin 被加载到的位置
OffsetOfKernelFile equ 0h  ;;Kernel.bin 被加载的偏移
