#include "x86.h"
#include "device.h"

#define INTERRUPT_GATE_32   0xE
#define TRAP_GATE_32        0xF

/* IDT表的内容 */
struct GateDescriptor idt[NR_IRQ];

/* 初始化一个中断门(interrupt gate) */
static void
set_intr(struct GateDescriptor *ptr, uint32_t selector, uint32_t offset, uint32_t dpl) {
	ptr->offset_15_0 = offset & 0xFFFF;
	ptr->segment = selector << 3;
	ptr->pad0 = 0;
	ptr->type = INTERRUPT_GATE_32;
	ptr->system = FALSE;
	ptr->privilege_level = dpl;
	ptr->present = TRUE;
	ptr->offset_31_16 = (offset >> 16) & 0xFFFF;
}

/* 初始化一个陷阱门(trap gate) */
static void
set_trap(struct GateDescriptor *ptr, uint32_t selector, uint32_t offset, uint32_t dpl) {
	ptr->offset_15_0 = offset & 0xFFFF;
	ptr->segment = selector << 3;
	ptr->pad0 = 0;
	ptr->type = TRAP_GATE_32;
	ptr->system = FALSE;
	ptr->privilege_level = dpl;
	ptr->present = TRUE;
	ptr->offset_31_16 = (offset >> 16) & 0xFFFF;
}

/* 声明函数，这些函数在汇编代码里定义 */
//void test1();
//void test2();
void sys();
void timer();
void irq_empty();
void test3();

void init_idt() {
	int i;

	/* 为了防止系统异常终止，所有irq都有处理函数(irq_empty)。 */
	for (i = 0; i < NR_IRQ; i ++) {
		set_trap(idt + i, SEG_KCODE, (uint32_t)irq_empty, DPL_KERN);
	}
    /*
     * init your idt here
     * 初始化 IDT 表, 为中断设置中断处理函数
     */
    set_trap(idt + 0x80, SEG_KCODE, (uint32_t)sys, DPL_USER);
    set_intr(idt + 0x20, SEG_KCODE, (uint32_t)timer, DPL_KERN);
    set_intr(idt + 0x2e, SEG_KCODE, (uint32_t)test3, DPL_USER);
 //   set_intr(idt + 0x41, SEG_KCODE, (uint32_t)test1, DPL_KERN);
 //   set_intr(idt + 0x42, SEG_KCODE, (uint32_t)test2, DPL_USER);

	/* 写入IDT */
	save_idt(idt, sizeof(idt));
}

