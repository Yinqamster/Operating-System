#ifndef __PCB_H__
#define __PCB_H__
#include "x86.h"
#include "device.h"
#include "common.h"
#include "sem.h"

#define MAX_PCB 10

extern void schedule();
extern void isBlocked();
extern void mysleep(unsigned);
extern void myfork();
extern void myexit(int);

enum State {Runnable, Blocked, Running, Dead};

typedef struct PCB{
	TrapFrame *tf;
    int state;
    int time_count;
    int sleep_time;
    unsigned int pid;
    char name[32];
    uint8_t stack[4096];
	semaphore *sem;
	struct PCB* prev;
	struct PCB* next;
}PCB;

extern PCB pcb_table[MAX_PCB];
extern PCB idle;
extern PCB *current;
extern PCB *Ready;
extern PCB *Sleep;
extern PCB *Free;

static inline void
list_add(PCB *prev, PCB *next, PCB *data) {
	assert(data != NULL);
	data->prev = prev;
	data->next = next;
	if (prev != NULL) prev->next = data;
	if (next != NULL) next->prev = data;
}

static inline void
list_add_before(PCB *list, PCB *data) {
	assert(list != NULL);
	list_add(list->prev, list, data);
}

static inline void
list_add_after(PCB *list, PCB *data) {
	assert(list != NULL);
	list_add(list, list->next, data);
}

static inline void
list_del(PCB *data) {
	assert(data != NULL);
	PCB *prev = data->prev;
	PCB *next = data->next;
//	assert(prev != NULL && prev->next == data);
//	assert(next != NULL && next->prev == data);
	if (prev != NULL) prev->next = next;
	if (next != NULL) next->prev = prev;
}

static inline void
list_init(PCB *list) {
	assert(list != NULL);
	list->prev = list->next = list;
}

static inline int
list_empty(PCB *list) {
	assert(list != NULL);
	return list == list->next;
}


#endif
