#
# NLOS Kernel makefile
# John Hansen
#

CC = i686-elf-gcc
CFLAGS = -ffreestanding -Wall -Wextra -Werror \
-fstack-protector -fstack-protector-strong -fstack-protector-all \
-Wnull-dereference \
-fno-delete-null-pointer-checks \
-ggdb \
-isystem ../sysroot/usr/include
ASM = nasm
ASMFLAGS = -f elf -ggdb
LDFLAGS = -nostdlib -Wall -Wextra -L../sysroot/usr/lib -lc

OBJECTS = boot.o \
kernel.o \
terminal.o \
idt.o \
queue.o \
keyboard.o \
isr.o \
paging.o \
liballoc.o \
liballoc_hooks.o \
process.o \
syscalls.o

.SUFFIXES: .kernel .o .c .asm .d

all: nlos.kernel

-include $(OBJECTS:.o=.d)

nlos.kernel: $(OBJECTS) linker.ld
	$(CC) -T linker.ld -o $@ $^ $(LDFLAGS)
	grub-file --is-x86-multiboot $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f nlos.kernel
	rm -f *.o
	rm -f *.d
