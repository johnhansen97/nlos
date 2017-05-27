#
# MyOS Kernel makefile
# John Hansen
#

CC = i686-elf-gcc
CFLAGS = -ffreestanding -O2 -Wall -Wextra -Werror \
-fstack-protector -fstack-protector-strong -fstack-protector-all \
-Wnull-dereference \
-fno-delete-null-pointer-checks
ASM = nasm
ASMFLAGS = -f elf
LDFLAGS = -nostdlib -Wall -Wextra

OBJECTS = boot.o \
kernel.o \
terminal.o \
idt.o \
queue.o \
keyboard.o \
isr.o \
paging.o

.SUFFIXES: .kernel .o .c .asm .d

all: myos.kernel

-include $(OBJECTS:.o=.d)

myos.kernel: $(OBJECTS) linker.ld
	$(CC) -T linker.ld -o $@ $^ $(LDFLAGS)
	grub-file --is-x86-multiboot $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f myos.kernel
	rm -f *.o
	rm -f *.d
