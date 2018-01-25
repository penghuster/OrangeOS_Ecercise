BOOT:=boot.asm
LDR:=loader.asm

BOOT_BIN:=$(subst .asm,.bin,$(BOOT))
LDR_BIN:=$(subst .asm,.bin,$(LDR))



.PHONY : everything

everything : $(BOOT_BIN) $(LDR_BIN)
	dd if=$(BOOT_BIN) of=b.img bs=512 count=1 conv=notrunc
	sudo mount -o loop b.img /mnt  
	sudo cp $(LDR_BIN) /mnt -v
	sudo umount /mnt 

$(BOOT_BIN) : $(BOOT)
	nasm $< -o $@

$(LDR_BIN) : $(LDR)
	nasm $< -o $@

clean :
	rm -rf $(BOOT_BIN) $(LDR_BIN)
