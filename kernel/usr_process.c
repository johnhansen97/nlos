#include <stdint.h>
#include "terminal.h"

extern terminal_t std;

__attribute__ ((aligned (4096)))
void usr_process(void) {
  void  (*print_str_absolute)(terminal_t *, const char *);
  print_str_absolute = print_str;
  print_str_absolute(&std, "Hello from userspace!");
  while (1) {}
}
