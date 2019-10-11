#include "common.h"
#include "x86.h"
#include "device.h"

extern void enter_user_space(void);

void
kentry(void) {
	init_serial();			//初始化串口输出
    init_idt();
	init_intr();
    init_seg();
    load_umain();
    enter_user_space();
    return;
    while(1);
	assert(0);
}
