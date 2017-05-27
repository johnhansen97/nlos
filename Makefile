#
# MyOS makefile
# John Hansen
#

.SUFFIXES: .iso

all: myos.iso

myos.iso: isodir/boot/grub/grub.cfg
	make -C kernel
	cp $< isodir/boot/myos.kernel
	grub-mkrescue -o myos.iso isodir

.PHONY: clean

clean:
	make -C kernel clean
	rm -f isodir/boot/myos.kernel
	rm -f myos.iso
