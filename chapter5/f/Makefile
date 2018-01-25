##################################################
# Makefile
##################################################

BOOT:=boot.asm
LDR:=loader.asm
ASM_SRC:=kernel.asm string.asm kliba.asm
C_SRC:=start.c

ASM_OBJ:=$(subst .asm,.o,$(ASM_SRC))
C_OBJ:=$(subst .c,.o,$(C_SRC))

BOOT_BIN:=$(subst .asm,.bin,$(BOOT))
LDR_BIN:=$(subst .asm,.bin,$(LDR))
KERNEL_BIN:=kernel.bin


IMG:=b.img
FLOPPY:=/mnt/

.PHONY : everything

everything : $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN)
	dd if=$(BOOT_BIN) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(IMG) $(FLOPPY)
	sudo cp $(LDR_BIN) $(FLOPPY) -v
	sudo cp $(KERNEL_BIN) $(FLOPPY) -v
	sudo umount $(FLOPPY)

clean :
	rm -f $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN) *.o

$(BOOT_BIN) : $(BOOT)
	nasm $< -o $@

$(LDR_BIN) : $(LDR)
	nasm $< -o $@

#$(ASM_OBJ) : $(ASM_SRC)
#	nasm -f elf $< -o $@

$(C_OBJ) : $(C_SRC)
	gcc -c -o $@ $<

$(KERNEL_BIN) : $(C_OBJ) $(ASM_OBJ) 
	ld -s -Ttext 0x30400 -o $@  $(ASM_OBJ) $(C_OBJ) 

%.o : %.asm
	nasm -f elf $< -o $@

