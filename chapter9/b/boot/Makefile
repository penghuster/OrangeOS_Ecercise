
#Programs, flags, etc 
ASM = nasm 
ASMFLAGS = -I./include/

#This Program 
TARGET = boot.bin loader.bin 

#All Phony Targets 
.PHONY: everything clean all 

#Default starting position 
everything: $(TARGET)

clean:
	rm -f $(TARGET)


all : clean everything


boot.bin : boot.asm include/loader.inc include/fat12hdr.inc
	$(ASM) $(ASMFLAGS) -o $@ $<


loader.bin : loader.asm include/loader.inc include/fat12hdr.inc include/pm.inc include/lib.inc
	$(ASM) $(ASMFLAGS) -o $@ $<
