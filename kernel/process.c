#include <stdint.h>
#include <string.h>
#include "paging.h"
#include "process.h"
#include "liballoc.h"
#include "kernel.h"
#include "inline_asm.h"

extern page_dir_t boot_page_directory;
extern uintptr_t upper_half;

extern void usr_process(void);

static uint32_t next_pid;

/**
 * Add a new page table to the process page tree.
 * @param p the process whose address space will be modified.
 * @param index the index of the new page table.
 */
static void process_new_pt(process_t *p, uint32_t index) {
  if (index >= 768) {
    kernel_panic_message("Process_new_pt: invalid index");
  }

  uintptr_t physical = page_frame_alloc(1);
  uintptr_t virtual = addr_block_remove(1);

  page_map(virtual, physical);

  p->page_tables[index] = (page_table_t *)virtual;
  p->page_dir_virtual->entries[index] = (physical & PAGE_DIR_ENTRY_ADDR)
    | PAGE_DIR_ENTRY_USER
    | PAGE_DIR_ENTRY_RW
    | PAGE_DIR_ENTRY_PRES;

  if (read_cr3() == p->page_dir_physical) {
    write_cr3(read_cr3());
  }
}

/**
 * Switch to process address space, load text, and init default
 * thread stack.
 * @param p process
 */
static void process_init_mem(process_t *p) {
  unsigned int i;
  uint32_t *stack = (uint32_t *)upper_half - 13;
  //Switch to process address space
  write_cr3(p->page_dir_physical);

  //Load process text
  process_map_page(p, PROCESS_TEXT_OFFSET, page_frame_alloc(1));
  memcpy((void *)PROCESS_TEXT_OFFSET, &usr_process, 4096);

  for (i = upper_half - 0x1000; i >= upper_half - THREAD_STACK_SIZE * 0x1000; i -= 0x1000) {
    process_map_page(p, i, page_frame_alloc(1));
  }

  stack[0] = 0;
  stack[1] = 0;
  stack[2] = upper_half;
  stack[3] = upper_half - 20;
  stack[4] = 0;
  stack[5] = 0;
  stack[6] = 0;
  stack[7] = 0;
  stack[8] = PROCESS_TEXT_OFFSET;
  stack[9] = 0x23;
  stack[10] = 0x200002;
  stack[11] = (uintptr_t)upper_half;
  stack[12] = 0x2b;
  p->thread_list->stk_ptr = (uintptr_t)upper_half - 52;
}

/**
 * Map a physical page to a process address space.
 * @param *p the process whose address space will be modified.
 * @param virtual_addr the virtual address to be mapped.
 * @param physical_addr the physical address to be mapped.
 */
void process_map_page(process_t *p, uintptr_t virtual_addr, uintptr_t physical_addr) {
  uint32_t page_dir_index = (virtual_addr & 0xFFC00000) >> 22;
  uint32_t page_table_index = (virtual_addr & 0x003FF000) >> 12;
  uint32_t entry = (physical_addr
		    & PAGE_TABLE_ENTRY_ADDR)
    | PAGE_TABLE_ENTRY_USER
    | PAGE_TABLE_ENTRY_RW
    | PAGE_TABLE_ENTRY_PRES;

  if (p->page_tables[page_dir_index] == 0) {
    process_new_pt(p, page_dir_index);
  }

  p->page_tables[page_dir_index]->entries[page_table_index] = entry;
  
  invlpg(virtual_addr);
}

/**
 * Initialize a given process.
 * The process will be initialized with its own page_dir.
 * @param *p pointer to the process
 * @param *name the name of the process
 */
void init_process(process_t *p, const char *name) {
  //Set up process_t struct
  p->pid = next_pid++;
  p->name[0] = name - name; //TODO: use strncpy
  p->page_dir_virtual = (page_dir_t *)addr_block_remove(1);
  p->page_dir_physical = page_frame_alloc(1);
  page_map((uintptr_t)p->page_dir_virtual, p->page_dir_physical);
  p->status = 0;

  //Map kernel PTs to process PD
  unsigned int i;
  for (i = 768; i < 1024; i++) {
    p->page_dir_virtual->entries[i] = boot_page_directory.entries[i];
  }

  //Init default thread
  p->thread_list = malloc(sizeof(thread_t));

  p->thread_list->tid = 0;
  p->thread_list->next_thread = 0;
  p->thread_list->stk_ptr = upper_half;

  //Init process memory
  process_init_mem(p);
}
