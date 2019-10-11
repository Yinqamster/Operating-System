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
	while (p != NULL) {
		p->sleep_time--;
		if (p->sleep_time <= 0) {
			p->state = Runnable;
			p->time_count = 10;
			s = p;
			if (s->next == NULL) {
				p = p->next;
				list_del(s);
				Sleep = NULL;
			}
			else {		
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
		else
			p = p->next;
	}
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
	if (Ready == NULL) Ready = p;
	else {
		while(q->next != NULL)
			q = q->next;
		list_add_after(q,p);
	}	
}

void mysleep(unsigned time) {
	PCB *p;
	current->sleep_time = time * HZ;
	current->state = Blocked;
	p = Sleep;
	if (current->next == NULL) {
		list_del(current);
		Ready = NULL;
	}
	else if (current->next != NULL) {
		Ready = current->next;
		list_del(current);
		current->next = NULL;
	}
	if (Sleep == NULL) {
		Sleep = current;
		Sleep->next = NULL;
	}
	else {
		while (p->next != NULL)
			p = p->next;
		list_add_after(p,current);
	}
	if (Ready == NULL) {
		current = &idle;
	}
	else {
		current = Ready;
		current->time_count = 10;
		current->state = Running;
		tss.esp0 = (int)current->stack + 4096;
	}
}

void myexit(int sig) {
	PCB* p;
	p = Free;
	current->state = Dead;
	if (current->next == NULL) {
		list_del(current);
		while (p->next != NULL)
			p = p->next;
		list_add_after(p,current);
		Ready = NULL;
		current = &idle;
	}
	else {
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
	if (current == &idle) {
		if (Ready != NULL) {
			current = Ready;
			current->time_count = 10;
			current->state = Running;
			tss.esp0 = (int)current->stack + 4096;
		}
		return;
	}
	if (current->time_count == 0) {
		PCB *p = current;
		if (p->state == Running)
			p->state = Runnable;
		PCB *q = Ready;
		while(q->next != NULL)
			q = q->next;
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

