#include "pcb.h"
#include "sem.h"
semaphore s;
extern TSS tss;
void W() {
	current->sleep_time = 0xffffffff;
	current->state = Blocked;
	PCB* p = s.list;

	if (current->next == NULL) {
		list_del(current);
		Ready = NULL;
	}
	else if (current->next != NULL) {
		Ready = current->next;
		list_del(current);
		current->next = NULL;
	}

	if (s.list == NULL) {
//		putchar('c');
		s.list = current;
		s.list->next = NULL;
	}
	else {
//		putchar('d');
		while(p->next != NULL)
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

void R() {
//	putchar('b');
	PCB *p = s.list;
	if (s.list->next == NULL) {
//	putchar('f');
		list_del(p);
		s.list = NULL;
	}
	else {
//	putchar('g');
		s.list = s.list->next;
		list_del(p);
	}
	
	p->state = Runnable;
	PCB *q = Ready;
	if (Ready == NULL)
		Ready = p;
	else {
		while(q->next != NULL)
			q = q->next;
		list_add_after(q, p);
	}
	current = Ready;
	current->state = Running;
	current->time_count = 10;

}
void P() {
//	putchar('e');
	s.value--;
	if (s.value < 0)
		W();	
}
void V() {
//	putchar('a');
	s.value++;
	if (s.value <= 0)
		R();
}
void createsem() {
	s.value = 0;
	s.state = Enable;
}
void destroysem() {
	s.state = Disable;
}


