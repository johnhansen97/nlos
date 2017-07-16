#
# MyOS makefile
# John Hansen
#

.SUFFIXES: .iso

all: myos.iso

myos.iso: sysroot/boot/grub/grub.cfg kernel usr_process
	cp kernel/myos.kernel sysroot/boot/myos.kernel
	grub-mkrescue -o myos.iso sysroot

.PHONY: clean kernel usr_process

usr_process:
	make -C user_process install

kernel:
	make -C libc install_headers
	make -C libc install_lib
	make -C kernel

clean:
	make -C libc clean
	make -C kernel clean
	make -C user_process clean
	rm -f sysroot/usr/include/*.h
	rm -f sysroot/usr/lib/*.a
	rm -f sysroot/boot/myos.kernel
	rm -f myos.iso
