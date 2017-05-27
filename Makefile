#
# MyOS makefile
# John Hansen
#

.SUFFIXES: .iso

all: myos.iso

myos.iso: isodir/boot/grub/grub.cfg kernel
	cp kernel/myos.kernel isodir/boot/myos.kernel
	grub-mkrescue -o myos.iso isodir

.PHONY: clean kernel

kernel:
	make -C kernel

clean:
	make -C kernel clean
	rm -f isodir/boot/myos.kernel
	rm -f myos.iso
