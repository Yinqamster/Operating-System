QEMU=qemu-system-i386

os.img:
	@cd bootloader; make
	@cd app; make
	cat bootloader/bootblock app/app > os.img

# 定义的一些伪目标
.PHONY: play clean debug
play: os.img
	$(QEMU) os.img

debug: os.img
	$(QEMU) —s -S os.img

clean:
	@cd bootloader; make clean
	@cd app; make clean
	rm -f os os.img
