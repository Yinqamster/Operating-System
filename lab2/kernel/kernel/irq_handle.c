#include "x86.h"
#include "device.h"

void do_syscall(struct TrapFrame *tf);
extern void putchar(char);

void
irq_handle(struct TrapFrame *tf) {
    /*
     * 中断处理程序
     */
//	putchar('a');assert(0);
    switch(tf->irq) {
	case 0x80:do_syscall(tf);break;
        case 1000:break;
        case 1001:break;
        default:break;
    }
}


void do_syscall(struct TrapFrame *tf) {
	switch(tf->eax) {
		case 4: {
			//int len = tf->edx;
			//char *buf = (void *)tf->ecx;
			putchar(tf->ecx);
		}break;
		default:assert(0);
	}
}
