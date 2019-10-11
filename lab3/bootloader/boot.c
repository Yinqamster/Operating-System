#include "boot.h"
#include "elf.h"

#define SECTSIZE 512
#define KOFFSET 0xC0000000
static void readsect(void * src, int offset);
static void readseg(unsigned char *, int, int);

static void myprint(char *str, short* addr) {
//	short* addr=0xb8000;
	int i;
	for(i=0;str[i]!='\0';i++){
		*addr=str[i]+0x0c00;
		addr++;
	}
}

void bootmain(void)
{
	char str1[] = "Loading...";
	char str2[] = "Execute...";
	char str3[] = "Back to boot";
//	myprint(str1,0xb80a0);
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;

/*	unsigned char buf[SECTSIZE];

	readsect(buf, 1);

	elf = (void*)buf;

	int i;
	for(i = 0; i < elf->phnum; i++) {
		unsigned char buf0[SECTSIZE];
		ph = (void*)buf + elf->phoff + elf->phentsize * i;
		if(ph->type == PT_LOAD) {
			int j;
			for(j = 0; j < ph->filesz; j++) {
				readsect((void*)buf0, (ph->off + j + 512) >> 9);
				unsigned char* s = (unsigned char*)ph->vaddr;
				s[j] = buf0[(ph->off + j + 512) & 511];
			}
			for(j = 0; j < ph->memsz - ph->filesz; j++) {
				unsigned char* t = (unsigned char*)(ph->vaddr + ph->filesz);
				t[j] = 0;
			}
//		memset((void*)ph->p_vaddr + ph->p_filesz, 0 ,ph->p_memsz - ph->p_filesz)
		}
	}*/
	unsigned char* pa, *i;

	elf = (struct ELFHeader*)0x8000;

	readseg((unsigned char*)elf, 4096, 0);

	ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph ++) {
		pa = (unsigned char*)(ph->paddr ); 
		readseg(pa, ph->filesz, ph->off);
		for (i = pa + ph->filesz; i < pa + ph->memsz; *i ++ = 0);
	}
//	myprint(str2, 0xb8140);
//	myprint(str3, 0xb8280);
	((void(*)(void))elf->entry)();
//	myprint(str3, 0xb8280);
	while(1);
}

static void
waitdisk(void) {
	while((in_byte(0x1F7) & 0xC0) != 0x40); /* 等待磁盘完毕 */
}

/* 读磁盘的一个扇区 */
static void
readsect(void *dst, int offset) {
	int i;
	waitdisk();
	out_byte(0x1F2, 1);
	out_byte(0x1F3, offset);
	out_byte(0x1F4, offset >> 8);
	out_byte(0x1F5, offset >> 16);
	out_byte(0x1F6, (offset >> 24) | 0xE0);
	out_byte(0x1F7, 0x20);

	waitdisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1F0);
	}
}

static void readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
}
