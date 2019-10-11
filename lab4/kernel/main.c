#include "common.h"
#include "x86.h"
#include "device.h"
#include "pcb.h"
extern void load_pcb();
extern void init_pcb();
extern void enter_user_space(void);
extern void init_timer();

void
kentry(void) {
	init_serial();			//初始化串口输出
	init_pcb();
	
    init_idt();
	init_intr();
	init_timer();
    init_seg();
    load_umain();
 //   enter_user_space();
	load_pcb();
	asm volatile("movl %%eax, %%esp"::"a"(idle.stack+4096));
    enable_interrupt();
    while(1) {
		wait_for_interrupt();
		putchar('^');
    }
	assert(0);
}
