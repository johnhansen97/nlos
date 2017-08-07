#
# NLOS makefile
# John Hansen
#

.SUFFIXES: .iso

all: nlos.iso

nlos.iso: sysroot/boot/grub/grub.cfg kernel user_programs
	cp kernel/nlos.kernel sysroot/boot/nlos.kernel
	grub-mkrescue -o nlos.iso sysroot

.PHONY: clean kernel user_programs

user_programs:
	make -C user_programs install

kernel:
	make -C libc install_headers
	make -C libc install_lib
	make -C kernel

clean:
	make -C libc clean
	make -C kernel clean
	make -C user_programs clean
	rm -f sysroot/usr/include/*.h
	rm -f sysroot/usr/lib/*.a
	rm -f sysroot/boot/nlos.kernel
	rm -f nlos.iso
