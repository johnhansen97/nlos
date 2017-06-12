#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define THREAD_STACK_SIZE 16

typedef struct thread {
  uint32_t tid;
  uintptr_t stk_ptr;
  struct thread *next_thread;
} thread_t;

typedef struct {
  uint32_t pid;
  char name[256];
  page_dir_t *page_dir_virtual;
  uintptr_t page_dir_physical;
  page_table_t *page_tables[768];
  thread_t *thread_list;
  uint32_t status;
} process_t;

void process_map_page(process_t *p, uintptr_t virtual_addr, uintptr_t physical_addr);
void init_process(process_t *p, const char *name);
#endif
