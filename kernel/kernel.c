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

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

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

  process_t *p = (process_t *)malloc(sizeof(process_t));
  init_process(p, "hi");

  asm volatile( "mov %0, %%esp;\npopa;\niret"
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
