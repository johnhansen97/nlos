#
# NLOS User Process Makefile
# John Hansen
#
# This program is for testing the functionality related to running
# user processes in NLOS.
#

CC = i686-elf-gcc
CFLAGS =
LD = i686-elf-ld
LDFLAGS = -Ttext 0x00100000 -e main --oformat binary
.PHONY: install clean

install: process.c
	$(CC) -c $<
	$(LD) -o ../sysroot/usr_process.bin process.o $(LDFLAGS)

clean:
	rm -f ../sysroot/usr_process.bin
	rm -f *.o
