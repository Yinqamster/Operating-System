#ifndef __SEM_H__
#define __SEM_H__

#include "pcb.h"

enum sem_state { Enable, Disable };

typedef struct semaphore{
		char state;
		int value;
		struct PCB* list;
} semaphore;

extern semaphore s;

extern void W();
extern void R();
extern void destroysem();
extern void createsem();
extern void P();
extern void V();

#endif
