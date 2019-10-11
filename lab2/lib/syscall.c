#include "lib.h"
#include "types.h"
#include "print.h"
#define sys_write 4
/*
 * io lib here
 * 库函数写在这
 */
static inline int32_t syscall(int num, int check, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;
	//Generic system call: pass system call number in AX
	//up to five parameters in DX,CX,BX,DI,SI
	//Interrupt kernel with T_SYSCALL
	//
	//The "volatile" tells the assembler not to optimize
	//this instruction away just because we don't use the
	//return value
	//
	//The last clause tells the assembler that this can potentially
	//change the condition and arbitrary memory locations.

	/*Lab2 code here
	  嵌入汇编代码，调用int $0x80
	 */
	asm volatile("int $0x80" : "=a"(ret): "a"(num), "b"(a1), "c"(a2), "d"(a3));
	return ret;
}


#define NULL (0)
#define SIZE 4096
void printf(const char *format,...){
	char* pArg = NULL;
	pArg = (char*)&format;
	pArg += sizeof(format);
	while (*format) {
		if (*format == '%') {
			switch(*(++format)) {
				case 'c':printch(*((char*)pArg));break;
				case 'd':printdec(*((int*)pArg));break;
				case 'x':printhex(*((unsigned*)pArg));break;
				case 's':printstr(*((char**)pArg));break;
			}
			pArg += sizeof(int);
		}
		else {
			printch(*format);
		}
		++format;    //here!!!
	}
} 



void printch(char ch) {
	syscall(4, 0, 0, ch, 0, 0, 0);
}
void printdec(int dec) {
	if (dec < 0) {
		if (dec == 0x80000000) {
			printstr("-2147483648");
			return;
		}
		printch('-');
		dec = -dec;
	}
	if (dec/10)
		printdec(dec/10);
	printch((char)(dec%10 + '0'));
}
void printstr (char* str) {
	while(*str) {
		printch(*str++);
	}
}
void printhex (unsigned hex) {
	if(hex/0x10) printhex(hex/0x10);
	if(hex%16 < 10) {
		printch((char)(hex%0x10 + '0'));
	}
	else {
		printch((char)(hex%0x10 + 'a' - 10));
	}
}

/*
void myprintf() {
	short* addr = 0xb81e0;
	char str[] = "Process:Hello world!";
	int i = 0;
	for(i=0;str[i]!='\0';i++) {
		*addr=str[i] + 0x0c00;
		addr++;
	}
}*/
