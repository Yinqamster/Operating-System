#include "x86.h"
#include "device.h"
#include <elf.h>
#include "pcb.h"
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

void load_pcb() {
	pcb_table[0].tf = (void*)(pcb_table[0].stack + 4096 - sizeof(struct TrapFrame));
	pcb_table[0].tf->ds = USEL(SEG_UDATA);
	pcb_table[0].tf->es = USEL(SEG_UDATA);
	pcb_table[0].tf->eip = elf->entry;
	pcb_table[0].tf->cs = USEL(SEG_UCODE);	
	pcb_table[0].tf->eflags = 0x202;  //0x202	
	pcb_table[0].tf->esp = 0x200000;
	pcb_table[0].tf->ss = USEL(SEG_UDATA);
	pcb_table[0].state = Runnable;
	pcb_table[0].time_count = 10;
	Ready = &pcb_table[0];
	Ready->prev = NULL;
	Ready->next = NULL;
//	current = &idle;
	Sleep = NULL;
	Free = NULL;
	int i;
	for (i = 1; i < MAX_PCB; i++) {
//putchar('4');

		PCB *p;
		if (pcb_table[i].state == Runnable) {
//putchar('6');
			if (Ready == NULL)
				Ready = &pcb_table[i];
			else {
				p = Ready;
				while(p->next != NULL)
					p = p->next;
				list_add_after(p, &pcb_table[i]);
			}
		}
		else if (pcb_table[i].state == Blocked) {
//putchar('7');
			if (Sleep == NULL)
				Sleep = &pcb_table[i];
			else {
				p = Sleep;
				while(p->next != NULL)
					p = p->next;
				list_add_after(p, &pcb_table[i]);
			}
		}
		else if (pcb_table[i].state == Dead) {
//putchar('8');
			if (Free == NULL)
				Free = &pcb_table[i];
			else {
				p = Free;
				while(p->next != NULL)
					p = p->next;
				list_add_after(p, &pcb_table[i]);
			}
		}
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
