##################################################
# Makefile for Orange's 
##################################################

# Entry point of Orange'S 
# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc 
ENTRYPOINT=0x30400

#Offset of entry point in kernel file 
#It depends on ENTRYPOINT
ENTRYOFFSET = 0x400 

IMG = b.img

#Programs, flags etc.
ASM = nasm 
DASM = ndisasm 
CC = gcc 
LD = ld 

ASMBFLAGS = -I boot/include/ 
ASMKFLAGS = -I include/ -f elf 
CFLAGS = -I include/ -c -fno-builtin -fno-stack-protector -fpack-struct -Wall
#LDFLAGS = -S -Ttext $(ENTRYPOINT)
#DASMFLAGS = -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)
LDFLAGS = -Ttext $(ENTRYPOINT) -Map krnl.map
DASMFLAGS = -D


#This Program 
ORANGESBOOT = boot/boot.bin boot/loader.bin
ORANGESKERNEL = kernel.bin
OBJS = kernel/kernel.o kernel/start.o kernel/i8259.o kernel/main.o \
	   kernel/global.o lib/klib.o kernel/protect.o  kernel/clock.o  \
	   kernel/proc.o kernel/syscall.o kernel/keyboard.o kernel/tty.o \
	   kernel/console.o kernel/printf.o kernel/vsprinf.o kernel/systask.o \
	   kernel/hd.o \
	   lib/kliba.o lib/string.o lib/misc.o lib/open.o lib/close.o lib/read.o \
	   lib/write.o lib/syslog.o lib/getpid.o lib/unlink.o \
	   fs/main.o fs/open.o fs/misc.o fs/disklog.o fs/link.o
DASMOUTPUT = kernel.bin.asm 

#all Phony Targets 
.PHONY: everything final image clean realclean disasm all buildimg 

all : realclean everything

final: all clean 

image : final buildimg


clean:
	rm -f $(OBJS)


realclean:
	rm -rf $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL)

disasm:
	$(disasm) $(DASMFLAGS) $(ORANGESKERNEL) > $(DASMOUTPUT)

#We assume that "a.img" exists in current folder
buildimg: boot/boot.bin boot/loader.bin kernel.bin
	dd if=./boot/boot.bin of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(IMG) /mnt 
	sudo cp -fv boot/loader.bin /mnt 
	sudo cp -fv kernel.bin /mnt 
	sudo umount /mnt 

boot/boot.bin: boot/boot.asm boot/include/loader.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin : boot/loader.asm boot/include/loader.inc boot/include/fat12hdr.inc \
	boot/include/pm.inc 
	$(ASM) $(ASMBFLAGS) -o $@ $<

i8259.o: kernel/i8259.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/type.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

$(ORANGESKERNEL) : $(OBJS)
	$(LD) $(LDFLAGS) -o $(ORANGESKERNEL) $(OBJS)

kernel/kernel.o : kernel/kernel.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/start.o : kernel/start.c include/type.h include/const.h include/protect.h \
	include/proto.h include/string.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/main.o : kernel/main.c include/type.h include/const.h include/protect.h \
	include/string.h include/proc.h include/proto.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o : kernel/clock.c include/type.h include/const.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/proc.o : kernel/proc.c include/type.h include/const.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/keyboard.o: kernel/keyboard.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/proc.h include/type.h \
	  include/proto.h include/string.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/proto.h include/string.h \
	  include/proc.h include/type.h include/global.h include/keyboard.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/string.h include/proc.h \
	  include/type.h include/proto.h include/tty.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o: kernel/printf.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/proc.h include/type.h \
	  include/tty.h include/string.h include/proto.h include/global.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/vsprinf.o: kernel/vsprinf.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/string.h include/proc.h \
	  include/type.h include/tty.h include/global.h include/keyboard.h \
	   include/proto.h
	$(CC) $(CFLAGS) -o $@ $<


kernel/systask.o: kernel/systask.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/tty.h include/proc.h \
	  include/type.h include/string.h include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/hd.o: kernel/hd.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/string.h include/proc.h \
	  include/type.h include/tty.h include/global.h include/keyboard.h \
	   include/proto.h include/hd.h
	$(CC) $(CFLAGS) -o $@ $<

kernel/syscall.o : kernel/syscall.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/global.o : kernel/global.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o : kernel/protect.c
	$(CC) $(CFLAGS) -o $@ $<



lib/kliba.o : lib/kliba.asm 
	$(ASM) $(ASMKFLAGS) -o $@ $< 


lib/string.o : lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/misc.o: lib/misc.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h
	$(CC) $(CFLAGS) -o $@ $<

lib/klib.o : lib/klib.c
	$(CC) $(CFLAGS) -o $@ $<

lib/open.o: lib/open.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/string.h include/fs.h \
	  include/proc.h include/type.h include/tty.h include/global.h \
	   include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

lib/close.o: lib/close.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/stdio.h include/protect.h include/string.h \
	  include/fs.h include/proc.h include/type.h include/tty.h \
	   include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

lib/read.o: lib/read.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/stdio.h include/protect.h include/string.h \
	  include/fs.h include/proc.h include/type.h include/tty.h \
	   include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

lib/write.o: lib/write.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/stdio.h include/protect.h include/string.h \
	  include/fs.h include/proc.h include/type.h include/tty.h \
	   include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

lib/syslog.o: lib/syslog.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/stdio.h include/protect.h include/string.h \
	  include/fs.h include/proc.h include/type.h include/tty.h \
	   include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

lib/getpid.o: lib/getpid.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/stdio.h include/protect.h include/string.h \
	  include/fs.h include/proc.h include/type.h include/tty.h \
	   include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

lib/unlink.o: lib/unlink.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/stdio.h include/protect.h include/string.h \
	  include/fs.h include/proc.h include/type.h include/tty.h \
	   include/global.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<


fs/main.o: fs/main.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/string.h include/fs.h \
	  include/proc.h include/type.h include/tty.h include/global.h \
	   include/proto.h include/hd.h
	$(CC) $(CFLAGS) -o $@ $<

fs/open.o: fs/open.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/string.h include/fs.h \
	  include/proc.h include/type.h include/tty.h include/global.h \
	   include/keyboard.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<

fs/misc.o: fs/misc.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/protect.h include/string.h include/fs.h \
	  include/proc.h include/type.h include/tty.h include/global.h \
	   include/keyboard.h include/proto.h include/hd.h
	$(CC) $(CFLAGS) -o $@ $<

fs/disklog.o: fs/disklog.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/stdio.h include/protect.h include/string.h \
	  include/fs.h include/proc.h include/type.h include/tty.h \
	   include/global.h include/keyboard.h include/proto.h include/hd.h
	$(CC) $(CFLAGS) -o $@ $<

fs/link.o: fs/link.c /usr/include/stdc-predef.h include/type.h \
	 include/const.h include/stdio.h include/protect.h include/string.h \
	  include/fs.h include/proc.h include/type.h include/tty.h \
	   include/global.h include/keyboard.h include/proto.h
	$(CC) $(CFLAGS) -o $@ $<


#BOOT:=boot.asm
#LDR:=loader.asm
#ASM_SRC:=kernel.asm string.asm kliba.asm
#C_SRC:=start.c

#ASM_OBJ:=$(subst .asm,.o,$(ASM_SRC))
#C_OBJ:=$(subst .c,.o,$(C_SRC))

#BOOT_BIN:=$(subst .asm,.bin,$(BOOT))
#LDR_BIN:=$(subst .asm,.bin,$(LDR))
#KERNEL_BIN:=kernel.bin


#IMG:=b.img
#FLOPPY:=/mnt/

#.PHONY : everything

#everything : $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN)
#	dd if=$(BOOT_BIN) of=$(IMG) bs=512 count=1 conv=notrunc
#	sudo mount -o loop $(IMG) $(FLOPPY)
#	sudo cp $(LDR_BIN) $(FLOPPY) -v
#	sudo cp $(KERNEL_BIN) $(FLOPPY) -v
#	sudo umount $(FLOPPY)

#clean :
#	rm -f $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN) *.o

#$(BOOT_BIN) : $(BOOT)
#	nasm $< -o $@

#$(LDR_BIN) : $(LDR)
#	nasm $< -o $@

#$(ASM_OBJ) : $(ASM_SRC)
#	nasm -f elf $< -o $@

#$(C_OBJ) : $(C_SRC)
#	gcc -c -o $@ $<

#$(KERNEL_BIN) : $(C_OBJ) $(ASM_OBJ) 
#	ld -s -Ttext 0x30400 -o $@  $(ASM_OBJ) $(C_OBJ) 

#%.o : %.asm
#	nasm -f elf $< -o $@

