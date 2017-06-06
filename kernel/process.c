#include <stdint.h>
#include "paging.h"
#include "process.h"
#include "liballoc.h"

extern page_dir_t boot_page_directory;

static uint32_t next_pid;


/** 
 * Initialize a given process.
 * The process will be initialized with its own page_dir.
 * @param *p pointer to the process
 * @param *name the name of the process
 */
void init_process(process_t *p, const char *name) {
  p->pid = next_pid++;
  p->name[0] = name - name; //TODO: use strncpy
  p->page_dir_virtual = (page_dir_t *)addr_block_remove(1);
  p->page_dir_physical = page_frame_alloc(1);
  page_map((uintptr_t)p->page_dir_virtual, p->page_dir_physical);

  int i;
  for (i = 768; i < 1024; i++) {
    p->page_dir_virtual->entries[i] = boot_page_directory.entries[i];
  }

  p->thread_list = (thread_t *)0;
  p->status = 0;
}
