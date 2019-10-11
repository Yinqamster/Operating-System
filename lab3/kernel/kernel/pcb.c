#include "x86.h"
#include "device.h"
#include "pcb.h"
#define HZ 10

PCB pcb_table[MAX_PCB];
PCB *Ready;
PCB *Sleep;
PCB *Free;
PCB idle;
extern TSS tss;
extern SegDesc gdt[NR_SEGMENTS];
PCB *current = &idle;
void init_pcb(void) {
    int i;
    for (i = 0; i < MAX_PCB; i++) {
		pcb_table[i].state = Dead;
		pcb_table[i].time_count = 0;
		pcb_table[i].sleep_time = 0;
		pcb_table[i].pid = i;
    }
}

void isBlocked() {
//putchar('^');
	PCB *p,*q,*s;
	p = Sleep;
/*	if (p == NULL)
		return;
	if (p->next == NULL) {
		p->sleep_time--;
		if (p->sleep_time <= 0) {
			p->state = Runnable;
			p->time_count = 10;
	}*/
	while (p != NULL) {
//putchar('^');
	//	if (p->state == Blocked) {
		p->sleep_time--;
		if (p->sleep_time <= 0) {
//		putchar('&');
			p->state = Runnable;
			p->time_count = 10;
			s = p;
			if (s->next == NULL) {
				p = p->next;
				list_del(s);
				Sleep = NULL;
		//		return;
			}
			else {
			//	s = p;		
				p = p->next;
				list_del(s);
				s->next = NULL;
				Sleep = p;
			}
			q = Ready;
			if (Ready == NULL)
				Ready = s;
			else {
				while(q->next != NULL)
					q = q->next;
				list_add_after(q,s);
			}
		}
	//	}
		else
			p = p->next;
	}
/*	if (p->next == NULL) {
		if (p->state == Blocked) {
			p->sleep_time--;
			if (p->sleep_time <= 0)
				p->state = Runnable;
		}
	}*/
}

void myfork() {
	gdt[6] = SEG(STA_X|STA_R, 0x400000, 0x200000, DPL_USER);
	gdt[7] = SEG(STA_W, 0x400000, 0x200000, DPL_USER);
	
	int i;
	for (i = 0; i < 0x200000; i++)
		*(char*)(0x400000+i) = *(char*)(0x200000+i);
	
	PCB *p = Free, *q = Ready;//, *s = Free->next;
	if (p == NULL)
		return;
	while(p->next != NULL)
		p = p->next;
	list_del(p);
	(*p) = (*current);
	p->tf = (void*)(((uint32_t)p - (uint32_t)current) + (uint32_t)current->tf);	
	p->tf->ds = (USEL(7));
	p->tf->es = (USEL(7));
	p->tf->cs = (USEL(6));
	p->tf->ss = (USEL(7));
	p->state = Runnable;
	p->time_count = 10;
	p->tf->eax = 0;
	p->pid = 1;
	current->tf->eax = 1;
	if (Ready == NULL) {
//	putchar('5');
		Ready = p;
	}
	else {
//	putchar('6');
		while(q->next != NULL)
			q = q->next;
		list_add_after(q,p);
	}
	
	
}

void mysleep(unsigned time) {
//	putchar('*');
	PCB *p;
	current->sleep_time = time * HZ;
	current->state = Blocked;
	p = Sleep;
	if (current->next == NULL) {
//putchar('6');
		list_del(current);
		Ready = NULL;
	}
	else if (current->next != NULL) {
//putchar('7');
		Ready = current->next;
		list_del(current);
		current->next = NULL;
	}
//	list_add_before(Sleep,current);
	if (Sleep == NULL) {
//putchar('1');	
		Sleep = current;
		Sleep->next = NULL;
	}
	else {
//putchar('2');	//list_add_before(Sleep,current);
	//	Sleep = current;
		while (p->next != NULL)
			p = p->next;
		list_add_after(p,current);
	}
	if (Ready == NULL) {
//putchar('3');	
		current = &idle;
	}
	else {
//putchar('4');	
		current = Ready;
		current->time_count = 10;
		current->state = Running;
		tss.esp0 = (int)current->stack + 4096;
	}

//	schedule();
}

void myexit(int sig) {
//	putchar('q');
	PCB* p;
	p = Free;
	current->state = Dead;
	if (current->next == NULL) {
//putchar('1');
		list_del(current);
		while (p->next != NULL)
			p = p->next;
		list_add_after(p,current);
		Ready = NULL;
		current = &idle;
	}
	else {
//putchar('2');
		Ready = current->next;
		list_del(current);
		while (p->next != NULL)
			p = p->next;
		list_add_after(p,current);
		current = Ready;
		current->state = Running;
		current->time_count = 10;
		tss.esp0 = (int)current->stack + 4096;
	}
}




void schedule() {
//	asm volatile("cli");
//	isBlocked();
//putchar('.');
	if (current == &idle) {
		if (Ready != NULL) {
//putchar('5');	
			current = Ready;
			current->time_count = 10;
			current->state = Running;
			tss.esp0 = (int)current->stack + 4096;
		}
//		if ( Sleep != NULL){
//			putchar('^');
//		}
//		else if {
//		}
		return;
	}

	if (current->time_count == 0) {
//		putchar('@');
		PCB *p = current;
		if (p->state == Running)
			p->state = Runnable;
		PCB *q = Ready;
		while(q->next != NULL)
			q = q->next;
	//	Ready = p->next;
		if (p != q) {
			list_del(p);
			list_add_after(q, p);
			while(q->prev != NULL)
				q = q->prev;
		}
		Ready = q;
		current = Ready;
		current->state = Running;
		current->time_count = 10;
		
	}
return;















}
