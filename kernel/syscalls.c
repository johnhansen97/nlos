#include <stdint.h>
#include "syscalls.h"
#include "terminal.h"
#include "inline_asm.h"

extern terminal_t std;

void (*syscalls[2]) (uint32_t eax,
		     uint32_t ebx,
		     uint32_t ecx,
		     uint32_t edx) = {sys_exit,
				      sys_std_out};
/**
 * Called by isr80, executes the appropriate syscall
 * @param eax register eax
 * @param ebx register ebx
 * @param ecx register ecx
 * @param edx register edx
 */
void syscall(uint32_t eax,
	     uint32_t ebx,
	     uint32_t ecx,
	     uint32_t edx) {
  syscalls[eax](eax, ebx, ecx, edx);
}

void sys_exit(uint32_t eax __attribute__ ((unused)),
	      uint32_t ebx __attribute__ ((unused)),
	      uint32_t ecx __attribute__ ((unused)),
	      uint32_t edx __attribute__ ((unused))) {
  print_str(&std, "\nProcess terminated.");
  while (1) {
    hlt();
  }
}

void sys_std_out(uint32_t eax __attribute__ ((unused)),
		 uint32_t ebx,
		 uint32_t ecx __attribute__ ((unused)),
		 uint32_t edx __attribute__ ((unused))) {
  print_str(&std, (const char *) ebx);
}
