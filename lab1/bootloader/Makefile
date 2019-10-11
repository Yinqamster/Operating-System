bootblock: boot.c start.S asm.h boot.h
	gcc -c -m32 start.S
	gcc -c -m32 -O1 -fno-stack-protector boot.c
	ld -melf_i386 -e start -Ttext 0x7C00 start.o boot.o -o bootblock.o
	@objcopy -S -O binary -j .text bootblock.o bootblock
	@../utils/genboot.pl bootblock

clean:
	rm -rf bootblock *.o
