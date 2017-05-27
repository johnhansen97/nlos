#include <stdint.h>
#include "kernel.h"
#include "terminal.h"
#include "idt.h"
#include "keyboard.h"
#include "inline_asm.h"
#include "paging.h"

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

void multiboot_show(void) {
  print_str(&std, "Loading multiboot data structure from ");
  print_hex(&std, (uint32_t) multiboot_info);
  print_ch(&std, '\n');

  print_str(&std, "\tFlags: ");
  print_hex(&std, *multiboot_info);
  print_ch(&std, '\n');

  if (*multiboot_info & 1 << 0) {
    print_str(&std, "\tMemory flag is set\n");
    print_str(&std, "\t\tmem_lower: ");
    print_hex(&std, multiboot_info[1]);
    print_ch(&std, '\n');
    print_str(&std, "\t\tmem_upper: ");
    print_hex(&std, multiboot_info[2]);
    print_ch(&std, '\n');
  }

  if (*multiboot_info & 1 << 1) {
    print_str(&std, "\tBoot device flag is set\n");
    print_str(&std, "\t\tboot_device: ");
    print_hex(&std, multiboot_info[3]);
    print_ch(&std, '\n');
  }

  if (*multiboot_info & 1 << 2) {
    print_str(&std, "\tCMDLINE flag is set\n");
    print_str(&std, "\t\tcmdline: \'");
    print_str(&std, (char *)(multiboot_info[4]+0xC0000000));
    print_str(&std, "\'\n");
  }

  if (*multiboot_info & 1 << 3) {
    print_str(&std, "\tModules flag is set\n");
    print_str(&std, "\t\tmods_count: ");
    print_hex(&std, multiboot_info[5]);
    print_ch(&std, '\n');
    print_str(&std, "\t\tmods_addr: ");
    print_hex(&std, multiboot_info[6]);
    print_ch(&std, '\n');
  }

  if (*multiboot_info & 1 << 5) {
    print_str(&std, "\tSyms flag is set, format is ELF\n");
    print_str(&std, "\t\tnum: ");
    print_hex(&std, multiboot_info[7]);
    print_ch(&std, '\n');
    print_str(&std, "\t\tsize: ");
    print_hex(&std, multiboot_info[8]);
    print_ch(&std, '\n');
    print_str(&std, "\t\taddr: ");
    print_hex(&std, multiboot_info[9]);
    print_ch(&std, '\n');
    print_str(&std, "\t\tshndx: ");
    print_hex(&std, multiboot_info[10]);
    print_ch(&std, '\n');
  }

  if (*multiboot_info & 1 << 6) {
    print_str(&std, "\tMemory Map flag is set\n");
    print_str(&std, "\t\tmmap_length: ");
    print_hex(&std, multiboot_info[11]);
    print_ch(&std, '\n');
    print_str(&std, "\t\tmmap_addr: ");
    print_hex(&std, multiboot_info[12]);
    print_ch(&std, '\n');
  }
  
  terminal_mv_cursor(&std);
}

void kernel_main(void) {
  terminal_init(&std);

  idt_init(&idt);
  load_idt();

  init_paging();

  //multiboot_show();
  keyboard_init();

  uintptr_t p = palloc(16);
  print_hex(&std, p);
  *((uint32_t *)p) = 0xBADA55; 
  get_ch();

  
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
