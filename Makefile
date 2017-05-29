#
# MyOS makefile
# John Hansen
#

.SUFFIXES: .iso

all: myos.iso

myos.iso: sysroot/boot/grub/grub.cfg kernel
	cp kernel/myos.kernel sysroot/boot/myos.kernel
	grub-mkrescue -o myos.iso sysroot

.PHONY: clean kernel

kernel:
	make -C kernel

clean:
	make -C kernel clean
	rm -f sysroot/boot/myos.kernel
	rm -f myos.iso
