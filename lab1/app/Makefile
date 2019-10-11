app: main.c lib.h
	gcc -c -m32 main.c
	ld -melf_i386 -e main -Ttext 0x00100000 main.o -o app

clean:
	rm -rf app *.o