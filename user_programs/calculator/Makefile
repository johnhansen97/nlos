#
# NLOS Calculator Makefile
# John Hansen
#

CC = i686-elf-gcc
CFLAGS = -ggdb -Wall -Wextra -Werror  -ffreestanding
LD = i686-elf-ld
LDFLAGS = -Ttext 0x00100000 -e main --oformat binary

OBJECTS = calc.o

BINARY = calc.bin

.SUFFIXES: .o .c .bin .d
.PHONY: install clean

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $^

-include $(OBJECTS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

install: $(BINARY)
	mkdir -p ../../sysroot/bin
	cp -R $< ../../sysroot/bin/$<

clean:
	rm -f ../../sysroot/bin/$(BINARY)
	rm -f *.bin
	rm -f *.o
	rm -f *.d
