# 系统启动时，BIOS会完成初始化，然后将磁盘第一个512字节保存到物理内存0x00007C00
# 位置，然后设置%CS=0000h, %IP=7C00h

#include "asm.h"
# 从此开始是16位代码
.code16
.globl start
start:
    #real mode code here
	cli                         #clear interrupt
#	xorw	%ax, %ax
#	movw	%ax, %ds
#	movw	%ax, %es
#	movw	%ax, %ss
	mov		$0x3, %ax
	int		$0x10                  //delete background words
	mov		$0x2401, %ax
	int		$0x15               #open A20
	lgdt	gdtdesc             #use lgdt to load gdt
#	or		$0x1, %cr0          #PE->1
	mov		%cr0, %eax
	or		$0x1, %eax
	mov		%eax, %cr0			#PE->1
	ljmp	$GDT_ENTRY(1), $start32
loop:
	jmp		loop


#	 printf "Hello, World!"
#    movw    $hello, %ax
#    movw    %ax, %bp
#    movw    $13, %cx        #输出的字符串长度
#    movw    $0x1301, %ax    #AH=0x13 在Teletype模式下显示字符串
                            #AL=0x01 输出方式
#    movw    $0x000c, %bx    #BH=0x00 黑底, BL=0x0c 红字
#    movw    $0x0000, %dx    #输出坐标 行0,列0
#    int     $0x10           #BIOS中断 显示服务
#.loop:
#    jmp     .loop
#hello:      .string "Hello, World!"     #字符串内容


.code32
start32:
    #protection mode code here
#	movl	$((80 * 5 + 0) * 2), %edi
#	movb	$0x0c, %ah
#	movb	$72, %al
#	movw	%ax, %gs:(%edi)
	movw	$GDT_ENTRY(2), %ax
	movw 	$GDT_ENTRY(3), %ax
	mov 	%ax, %gs
#	movw	%ax, %ds
#	movw	%ax, %es
#	movw	%ax, %ss
	movl	$0x8000, %esp
	mov 	$Boot_hello, %esi
    	movl 	$0x00, %ecx
	call 	Print
	call	bootmain
loop32:
    jmp loop32

Boot_hello:
    .string "Boot: Hello, World!"

.global Print
Print:
    	push %edi
	push %ebx
    	movb $0x0c, %ah
    loop1:   
	movb 0x0(%esi), %al
	testb %al, %al
	jz loop2
	movw %ax, %gs:(%edi)
	add $2, %edi
	add $1, %esi
	jmp loop1
    loop2:
	pop %ebx
	pop %edi
	ret



# GDT
.p2align 2                         # 对齐
    #GDT definition here
gdt:    #NULL
//    .word    0,0
//    .byte    0,0,0,0
	SEG_NULLASM 
	SEG_ASM(0x0A, 0, 0Xfffff)  #cs
	SEG_ASM(0x02, 0, 0xfffff)  #ds
	SEG_ASM(0x02, 0xB8000, 0xfffff) #gs


gdtdesc:                           # GDT描述符
	.word   (gdtdesc - gdt - 1)    # GDT长度，留意地址运算
	.long   gdt                    # GDT地址

