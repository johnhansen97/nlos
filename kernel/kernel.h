#ifndef KERNEL_H
#define KERNEL_H

__attribute__((noreturn))
void kernel_panic_message(const char *c);
void kernel_debug_str(const char *s);
#endif
