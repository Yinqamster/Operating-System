#include "x86.h"
#include "device.h"
#include <elf.h>
#define SECTSIZE 512

SegDesc gdt[NR_SEGMENTS];       // the new GDT
TSS tss;
static struct ELFHeader *elf;
void
readsect(void *dst, int offset);
void
init_seg() { // setup kernel segements
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0x200000,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0x200000,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
    gdt[SEG_TSS].s = 0;
	set_gdt(gdt, sizeof(gdt));

    /*
	 * 初始化TSS
	 */
	asm volatile("movl %%esp,%0":"=m"(tss.esp0));
	tss.ss0=KSEL(SEG_KDATA);
	asm volatile("ltr %%ax"::"a"(KSEL(SEG_TSS)));
	/*设置正确的段寄存器*/
	asm volatile("movw %%ax,%%es":: "a" (KSEL(SEG_KDATA)));
	asm volatile("movw %%ax,%%ds":: "a" (KSEL(SEG_KDATA)));
	asm volatile("movw %%ax,%%ss":: "a" (KSEL(SEG_KDATA)));

	lldt(0);
}

void
enter_user_space(void) {
    /*
     * Before enter user space 
     * you should set the right segment registers here
     * and use 'iret' to jump to ring3
     * 进入用户空间
     */
	volatile int Eip=elf->entry;
	asm volatile("movw %%ax,%%es":: "a" (USEL(SEG_UDATA))); //es
        asm volatile("movw %%ax,%%ds":: "a" (USEL(SEG_UDATA))); //ds
	
	asm volatile("pushl %0"::"a"(USEL(SEG_UDATA)));		//ss
	asm volatile("pushl $0x300000");			//esp
	asm volatile("pushl $0x202");				//eflags
	asm volatile("pushl %0"::"a"(USEL(SEG_UCODE)));		//cs
	asm volatile("pushl %0"::"a"(Eip));			//eip
	asm volatile("iret");
}

#define SIZE 8
#define START 201
static char buf[4096];

void
load_umain(void) {
    /*
     * Load your app here
     * 加载用户程序
     */
	struct ProgramHeader *ph, *eph;
	unsigned char *pa, *epa;
	unsigned char *k;
	elf = (struct ELFHeader*)buf;
	pa = (unsigned char*)elf;
	epa = pa + 4096;
	int i = 0;
	for (; i < SIZE; ++i) {
		readsect(buf + 512 * i, START + i);
	};
	 ph = (struct ProgramHeader *)((uint32_t)elf + elf->phoff);
	eph = ph + elf->phnum;
	for (;ph < eph; ph++) {
		pa = (unsigned char*)(ph->paddr)+0x200000;
		epa = pa + ph->filesz;
		int offset = (ph->off/SECTSIZE) + START;
		for (;pa<epa; pa += SECTSIZE,offset++)
			readsect((void*)pa,offset);
		for (k = pa + ph->filesz; k < pa + ph->memsz; *k ++ = 0);
	}
}

void
waitdisk(void) {
	while((in_byte(0x1F7) & 0xC0) != 0x40); 
}

void
readsect(void *dst, int offset) {
	int i;
	waitdisk();
	out_byte2(0x1F2, 1);
	out_byte2(0x1F3, offset);
	out_byte2(0x1F4, offset >> 8);
	out_byte2(0x1F5, offset >> 16);
	out_byte2(0x1F6, (offset >> 24) | 0xE0);
	out_byte2(0x1F7, 0x20);

	waitdisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1F0);
	}
}
