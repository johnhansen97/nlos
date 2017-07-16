#
# MyOS User Process Makefile
# John Hansen
#
# This program is for testing the functionality related to running
# user processes in MyOS.
#

.PHONY: install clean

install: process.asm
	nasm -f bin -o ../sysroot/usr_process.bin $<

clean:
	rm -f ../sysroot/usr_process.bin
