# 中断和异常处理函数的入口
## 中断处理函数会在IDT中为相应的中断/异常设置处理程序
# 中断/异常的行为参见i386手册

#.globl test1; test1: pushl $1000; jmp asm_do_irq
#.globl test2; test2: pushl $1001; jmp asm_do_irq
.globl sys;   sys:   pushl $0x80;   jmp asm_do_irq
.globl timer; timer: pushl $0x20;   jmp asm_do_irq
.globl test3; test3: pushl $0x2e;   jmp asm_do_irq
.globl irq_empty; irq_empty: pushl $-1; jmp asm_do_irq
# ！调用C编写的中断处理程序(见irq/irq_handle.c)！
.globl asm_do_irq
.extern irq_handle
# 在 ring3 用户空间下发生中断时需保存现场信息, 而陷入内核时要首先设置正确的段寄存器
asm_do_irq:
	cli
	pushal
	pushl %es
	pushl %ds
        movw $16,%ax
	movw %ax,%ds
	movw %ax,%es
	#movw $48,%ax
	#movw %ax,%gs

	pushl %esp
	call irq_handle
	#addl $4, %esp
	#movw $35,%ax
	#movw %ax,%ds

	mov (current), %eax
	mov (%eax), %esp

        
	popl %ds
	popl %es
	popal
	addl $4, %esp
	
	
	iret
