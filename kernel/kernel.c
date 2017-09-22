#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "terminal.h"
#include "idt.h"
#include "keyboard.h"
#include "inline_asm.h"
#include "paging.h"
#include "liballoc.h"
#include "process.h"
#include "elf.h"

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

extern process_t *current_process;

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __attribute__((noreturn)) __stack_chk_fail(void) {
  kernel_panic_message("STACK SMASHED");
  while (1) {
    continue;
  }
}

void load_idt(void);

uint32_t *multiboot_info;
uintptr_t upper_half;
uintptr_t kernel_physical_start;
uintptr_t kernel_virtual_start;
uintptr_t kernel_virtual_end;
uintptr_t kernel_physical_end;
terminal_t std;
idt_t idt;
int i = 0;

void isr0(void) {
  i++;
  //std.row = std.column = 0;
  //print_hex(&std, i);
}

void kernel_main(void) {
  terminal_init(&std);

  idt_init(&idt);
  init_paging();
  keyboard_init();

  load_idt();

  print_str(&std, "NLOS has booted.\n");
  print_str(&std, "Compiled on ");
  print_str(&std, __DATE__);
  print_str(&std, " at ");
  print_str(&std, __TIME__);
  print_str(&std, ".\n");

  process_t *p = (process_t *)malloc(sizeof(process_t));
  init_process(p, "hi");
  loadElf(*(uint32_t *)(multiboot_info[6] + upper_half),
	  ((uint32_t *)(multiboot_info[6] + upper_half))[1],
	  p);

  current_process = p;
  print_str(&std, "Running process:\n");
  asm volatile( "mov %0, %%esp;\nmov $0x2b, %%eax;\nmov %%eax, %%ds;\npopa;\niret"
		:
		:"r" (p->thread_list->stk_ptr)
		);
  

  
  while (1) {
    print_ch(&std, get_ch());
    terminal_mv_cursor(&std);
  }

  return;
}

void kernel_panic(void) {
  int i;

  std.row = std.column = 0;
  terminal_set_color(&std, VGA_COLOR_WHITE, VGA_COLOR_RED);
  for (i = 0; i < std.width * std.height; i++) {
    print_ch(&std, ' ');
  }
  std.row = std.column = 0;
  print_str(&std, "KERNEL PANIC!\n");
}

__attribute__ ((noreturn))
void kernel_panic_message(const char *s) {
  kernel_panic();

  print_str(&std, s);
  while (1) {
    asm ("cli");
    hlt();
  }
}

__attribute__ ((noreturn))
void kernel_panic_pf(uint32_t addr) {
  kernel_panic();

  print_str(&std, "Page fault at address: ");
  print_hex(&std, addr);
  while (1) {
    asm ("cli");
    hlt();
  }
}

/**
 * Print string to bochs terminal using port 0xE9 hack
 */
void kernel_debug_str(const char *s) {
  int i = 0;
  while(s[i]) {
    outb(0xE9, s[i]);
    i++;
  }
}
