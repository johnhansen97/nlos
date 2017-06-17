CC = i686-elf-gcc
CFLAGS = -ffreestanding -Wall -Wextra -Werror \
-fstack-protector -fstack-protector-strong -fstack-protector-all \
-ggdb \
-isystem ../sysroot/usr/include
AR = 

PREFIX = ../sysroot/usr

OBJS = string/memcpy.o

.SUFFIXES: .c .o .d

-include  $(OBJS:.o=.d)

libc.a: $(OBJS)
	ar rcs $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

.PHONY: install_headers install_lib clean

install_headers:
	mkdir -p $(PREFIX)/include
	cp -R include/. $(PREFIX)/include/.

install_lib:	libc.a
	mkdir -p $(PREFIX)/lib
	cp libc.a $(PREFIX)/lib/libc.a

clean:
	rm -f libc.a
	rm -f */*.o
	rm -f */*.d
