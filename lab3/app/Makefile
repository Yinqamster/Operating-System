CC = gcc
LD = ld
CFLAGS = -m32 -march=i386 -static -MD -std=gnu89 -ggdb \
		 -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
		 -Wall -Werror -O2 -I../lib
ASFLAGS = -ggdb -m32 -MD
LDFLAGS = -melf_i386
QEMU = qemu-system-i386

# 编译目标：src目录下的所有.c和.S文件
UCFILES = $(shell find ./ -name "*.c")
LCFILES = $(shell find ../lib -name "*.c")
UOBJS = $(UCFILES:.c=.o) $(LCFILES:.c=.o)

umain: $(UOBJS)
#	$(LD) $(LDFLAGS) -e uentry -Ttext 0x00200000 -o umain $(UOBJS)
	$(LD) $(LDFLAGS) -e uentry -Ttext 0x0 -o umain $(UOBJS)

clean:
	rm -rf $(UOBJS) $(UOBJS:.o=.d) umain
