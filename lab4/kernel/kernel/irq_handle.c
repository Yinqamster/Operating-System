#include "x86.h"
#include "device.h"
#include "pcb.h"
#include "sem.h"

void do_syscall(struct TrapFrame *tf);
extern void putchar(char);

void
irq_handle(struct TrapFrame *tf) {
    /*
     * 中断处理程序
     */
//	putchar('a');assert(0);
    current->tf = tf;
    switch(tf->irq) {
	case 0x80:do_syscall(tf);break;
        case 1000:break;
        case 1001:break;
	case 0x20:/*putchar('@');*/current->time_count--;isBlocked();
		break;
	case 0x2e:break;
        default:assert(0);
    }
    
    schedule();
}


void do_syscall(struct TrapFrame *tf) {
	switch(tf->eax) {
		case 4: {
			//int len = tf->edx;
			//char *buf = (void *)tf->ecx;
			putchar(tf->ecx);
		}break;
		case 1:{int sig = tf->ecx; myexit(sig);}break;
		case 2: {
//			putchar('P');
			myfork();
		}break;
		case 7:{ uint32_t time = tf->ecx; mysleep(time);}break;	
		case 8: {
			P();
			}break;
		case 9: {
			V();
			}break;
		case 10: {
			createsem();		
			}break;
		case 11:break;
		default:assert(0);
	}
}
