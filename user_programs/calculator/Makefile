#
# NLOS Calculator Makefile
# John Hansen
#

CC = i686-elf-gcc
CFLAGS = -ggdb -Wall -Wextra -Werror  -ffreestanding
LD = i686-elf-ld
LDFLAGS =

OBJECTS = calc.o

BINARY = calc

.SUFFIXES: .o .c .d
.PHONY: install clean

all: $(BINARY)

$(BINARY): $(OBJECTS) linker.ld
	$(LD) -o $@ -T linker.ld $(LDFLAGS) $^

-include $(OBJECTS:.o=.d)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

install: $(BINARY)
	mkdir -p ../../sysroot/bin
	cp -R $< ../../sysroot/bin/$<

clean:
	rm -f ../../sysroot/bin/$(BINARY)
	rm -f $(BINARY)
	rm -f *.o
	rm -f *.d
