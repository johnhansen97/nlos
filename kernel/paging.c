#include <stdint.h>
#include "paging.h"
#include "terminal.h"
#include "kernel.h"
#include "inline_asm.h"

extern terminal_t std;
extern uint32_t *multiboot_info;
extern uintptr_t kernel_virtual_end;
extern uintptr_t kernel_physical_end;

typedef struct {
  uint32_t entries[1024];
} page_table_t;

typedef struct {
  uint32_t entries[1024];
} page_dir_t;

typedef struct {
  int32_t next;
  int32_t prev;
  uintptr_t start;
  uint32_t size;
} addr_space_node_t;

static addr_space_node_t k_addr_space_list[0x20000];
static int32_t k_addr_space_head = -1;

extern page_dir_t boot_page_directory;
static page_dir_t *page_dir;

__attribute__ ((aligned (0x1000)))
static page_table_t page_tables[1024];

static uint32_t *page_frame_alloc_virtual;
static uint32_t *page_frame_alloc_physical;

static int32_t next_open_block_index(void) {
  int32_t i;
  for (i = 0; i >= 0; i++) {
    if (k_addr_space_list[i].size == 0) {
      return i;
    }
  }
  return 0;
}

static void merge_block_index(uint32_t i) {
  int prev = k_addr_space_list[i].prev;
  int next = k_addr_space_list[i].next;
  uintptr_t start = k_addr_space_list[i].start;
  uint32_t size = k_addr_space_list[i].size;

  if (prev >= 0) {
    if (k_addr_space_list[prev].start + (k_addr_space_list[prev].size << 12) == start) {
      start = k_addr_space_list[prev].start;
      size += k_addr_space_list[prev].size;
      k_addr_space_list[prev].size = 0;
      if (prev == k_addr_space_head) {
	k_addr_space_head = i;
      }
      prev = k_addr_space_list[prev].prev;
      if (prev >= 0) {
	k_addr_space_list[prev].next = i;
      }
    }
  }

  if (next >= 0) {
    if (start + (size << 12) == k_addr_space_list[next].start) {
      size += k_addr_space_list[next].size;
      k_addr_space_list[next].size = 0;
      next = k_addr_space_list[next].next;
      if (next >= 0) {
	k_addr_space_list[next].prev = i;
      }
    }
  }

  k_addr_space_list[i].next = next;
  k_addr_space_list[i].prev = prev;
  k_addr_space_list[i].start = start;
  k_addr_space_list[i].size = size;
}

/**
 * Starting address, size in pages
 */
void insert_addr_block(uintptr_t start, uint32_t size) {
  int i = next_open_block_index();
  int head = k_addr_space_head;
  int next, prev;

  if (size == 0) {
    kernel_panic_message("insert_addr_block: invalid size paramater");
  }

  if (start & 0xFFF) {
    kernel_panic_message("insert_addr_block: invalid start alignment");
  }

  k_addr_space_list[i].start = start;
  k_addr_space_list[i].size = size;

  //insert in empty list
  if (head < 0) {
    next = -1;
    prev = -1;
    k_addr_space_head = i;

    //insert in front of non-empty list
  } else if (k_addr_space_list[head].size > size) {
    prev = -1;
    next = head;
    k_addr_space_head = i;
    k_addr_space_list[next].prev = i;
    
    //insert in middle or end of list
  } else {
    prev = k_addr_space_head;
    //iterate through list
    while (k_addr_space_list[prev].start > start) {
      //end of list
      if (k_addr_space_list[prev].next < 0) {
	break;
      } else {
	prev = k_addr_space_list[prev].next;
      }
    }

    next = k_addr_space_list[prev].next;
    k_addr_space_list[prev].next = i;
    if (next >= 0) {
      k_addr_space_list[next].prev = i;
    }
  }

  k_addr_space_list[i].next = next;
  k_addr_space_list[i].prev = prev;
  merge_block_index(i);
}

uintptr_t addr_block_remove(uint32_t size) {
  int i = k_addr_space_head;
  uintptr_t ret = 0;

  while (i >= 0) {
    if (k_addr_space_list[i].size > size) {
      ret = k_addr_space_list[i].start;
      k_addr_space_list[i].start += size << 12;
      k_addr_space_list[i].size -= size;
      break;
    } 

    if (k_addr_space_list[i].size == size) {
      ret = k_addr_space_list[i].start;
      k_addr_space_list[i].size = 0;
      if (k_addr_space_list[i].prev >= 0) {
	k_addr_space_list[k_addr_space_list[i].prev].next =
	  k_addr_space_list[i].next;
      } else {
	k_addr_space_head = i;
      }

      if (k_addr_space_list[i].next >= 0) {
	k_addr_space_list[k_addr_space_list[i].next].prev =
	  k_addr_space_list[i].prev;
      }
      break;
    }
  }

  return ret;
}

void print_addr_blocks(void) {
  int i = k_addr_space_head;
  
  print_str(&std, "<addr_blocks>\n");
  while (i >= 0) {
    print_str(&std, "start: ");
    print_hex(&std, k_addr_space_list[i].start);
    print_str(&std, " size: ");
    print_hex(&std, k_addr_space_list[i].size);
    print_ch(&std, '\n');
    i = k_addr_space_list[i].next;
  }
  print_str(&std, "</addr_blocs>\n");
}


void init_paging(void) {
  page_dir = &boot_page_directory;

  //initialize virtual address space
  uint32_t kernel_memory_size = (0xFFFFFFFF - kernel_virtual_end + 1) >> 12;
  insert_addr_block(kernel_virtual_end, kernel_memory_size);

  //initialize physical address space
  uint32_t multiboot_memory = (multiboot_info[2] << 10) + 0x00100000;
  uint32_t physical_size = (multiboot_memory - kernel_physical_end) >> 12;
  page_frame_alloc_virtual = (uint32_t *) addr_block_remove(1);
  page_frame_free(kernel_physical_end, physical_size);
  /* page_frame_alloc_physical = (uint32_t *) kernel_physical_end; */
  /* page_frame_alloc_virtual = (uint32_t *) addr_block_remove(1); */
  /* page_map((uintptr_t)page_frame_alloc_virtual, (uintptr_t)page_frame_alloc_physical); */
  /* page_frame_alloc_virtual[0] = 0; */
  /* page_frame_alloc_virtual[1] = physical_size; */
}


uintptr_t page_frame_alloc(uint32_t n) {
  uintptr_t ret = (uintptr_t)page_frame_alloc_physical;
  uint32_t *next;
  uint32_t size;

  page_map((uintptr_t)page_frame_alloc_virtual, (uintptr_t)page_frame_alloc_physical);

  while (n != 0) {
    if (page_frame_alloc_virtual[1] > n) {
      next = (uint32_t *)page_frame_alloc_virtual[0];
      size = page_frame_alloc_virtual[1];
      ((uint32_t volatile *)page_frame_alloc_virtual)[0] = 0;
      ((uint32_t volatile *)page_frame_alloc_virtual)[1] = n;

      page_frame_alloc_physical = page_frame_alloc_physical + (n << 10);
      page_map((uintptr_t)page_frame_alloc_virtual, (uintptr_t) page_frame_alloc_physical);

      ((uint32_t volatile *)page_frame_alloc_virtual)[0] = (uint32_t)next;
      ((uint32_t volatile *)page_frame_alloc_virtual)[1] = size - n;
      break;

    } else if (page_frame_alloc_virtual[1] == n) {
      next = (uint32_t *)page_frame_alloc_virtual[0];
      size = page_frame_alloc_virtual[1];
      page_frame_alloc_virtual[0] = 0;

      page_frame_alloc_physical = next;
      page_map((uintptr_t)page_frame_alloc_virtual, (uintptr_t)page_frame_alloc_physical);
      break;

    } else {
      next = (uint32_t *)page_frame_alloc_virtual[0];
      size = page_frame_alloc_virtual[1];

      page_frame_alloc_physical = next;
      page_map((uintptr_t)page_frame_alloc_virtual, (uintptr_t)page_frame_alloc_physical);
      n -= size;
    }
  }
  return ret;

}

/**
 * Mark physical page frame as free
 * @param start starting address of page frame.
 * @param size size of page frame.
 */
void page_frame_free(uintptr_t start, uint32_t size) {
  uintptr_t next;

  //if there are no page frames
  if (page_frame_alloc_physical == 0) {
    page_frame_alloc_physical = (uint32_t *)start;
    page_map((uintptr_t)page_frame_alloc_virtual, (uintptr_t)page_frame_alloc_physical);
    page_frame_alloc_virtual[0] = 0;
    page_frame_alloc_virtual[1] = size;
    return;
  }

  page_map((uintptr_t)page_frame_alloc_virtual, (uintptr_t)page_frame_alloc_physical);
  next = page_frame_alloc_virtual[0];
  if (start + (size << 12) <= (uintptr_t)page_frame_alloc_physical) {
    page_map((uintptr_t)page_frame_alloc_virtual, start);
    page_frame_alloc_virtual[0] = (uintptr_t)page_frame_alloc_physical;
    page_frame_alloc_virtual[1] = size;
    if (start + (size << 12) == (uintptr_t)page_frame_alloc_physical) {
      page_frame_alloc_virtual[0] = next;
      page_map((uintptr_t)page_frame_alloc_virtual, (uintptr_t)page_frame_alloc_physical);
      size = page_frame_alloc_virtual[1];
      page_map((uintptr_t)page_frame_alloc_virtual, start);
      page_frame_alloc_virtual[1] += size;
    }
    page_frame_alloc_physical = (uint32_t *)start;
  } else {
    uintptr_t prev_physical = (uintptr_t)page_frame_alloc_physical;
    while (next < start && next != 0) {
      page_map((uintptr_t)page_frame_alloc_virtual, next);
      prev_physical = next;
      next = page_frame_alloc_virtual[0];
    }

    if (prev_physical + (page_frame_alloc_virtual[1] << 12) == start) {
      page_frame_alloc_virtual[1] += size;
    } else {
      page_frame_alloc_virtual[0] = start;
      page_map((uintptr_t)page_frame_alloc_virtual, start);
      page_frame_alloc_virtual[0] = next;
      prev_physical = start;
    }

    if (prev_physical + (page_frame_alloc_virtual[1] << 12) == next) {
      page_map((uintptr_t)page_frame_alloc_virtual, next);
      next = page_frame_alloc_virtual[0];
      size = page_frame_alloc_virtual[1];
      page_map((uintptr_t)page_frame_alloc_virtual, prev_physical);
      page_frame_alloc_virtual[0] = next;
      page_frame_alloc_virtual[1] += size;
    }
  }
    
}

/**
 * Dynamic page allocator for kernel.
 * Returns the starting address of n allocated pages in kernel memory (>0xC0000000).
 * @param n number of pages to allocate.
 * @return The virtual address of the new pages.
 */
uintptr_t palloc(uint32_t n) {
  uintptr_t ret;
  uintptr_t virtual_addr;
  uintptr_t physical_addr;
  uint32_t page_frame_size;
  uintptr_t next_page_frame;
  uint32_t i = 0;

  ret = virtual_addr = addr_block_remove(n);
  physical_addr = page_frame_alloc(n);
  do {
    page_map(virtual_addr, physical_addr);
    next_page_frame = *((uint32_t *)virtual_addr);
    page_frame_size = *((uint32_t *)virtual_addr + 1);
    virtual_addr += 0x1000;
    physical_addr += 0x1000;
    i++;
    while (i < page_frame_size) {
      page_map(virtual_addr, physical_addr);
      virtual_addr += 0x1000;
      physical_addr += 0x1000;
      i++;
    }
    physical_addr = next_page_frame;
  } while (next_page_frame);

  return ret;
}

/**
 * Map a virtual page to a physical page
 */
void page_map(uintptr_t virtual_addr, uintptr_t physical_addr) {
  uint32_t page_dir_index = (virtual_addr & 0xFFC00000) >> 22;
  uint32_t page_table_index = (virtual_addr & 0x003FF000) >> 12;
  uint32_t entry = (physical_addr
		    & PAGE_TABLE_ENTRY_ADDR)
    | PAGE_TABLE_ENTRY_USER
    | PAGE_TABLE_ENTRY_RW
    | PAGE_TABLE_ENTRY_PRES;

  // if page table entry is not present
  if (!(page_dir->entries[page_dir_index] & PAGE_DIR_ENTRY_PRES)) {
    page_dir->entries[page_dir_index] = 
      (((uintptr_t)&page_tables[page_dir_index] - 0xC0000000)
      & PAGE_DIR_ENTRY_ADDR)
      | PAGE_DIR_ENTRY_USER
      | PAGE_DIR_ENTRY_RW
      | PAGE_DIR_ENTRY_PRES;
  }
  page_tables[page_dir_index].entries[page_table_index] = entry;
  invlpg(virtual_addr);
}


/**
 * Unmap a virtual page from the page table
 */
void page_unmap(uintptr_t virtual_addr) {
  uint32_t page_dir_index = (virtual_addr & 0xFFC00000) >> 22;
  uint32_t page_table_index = (virtual_addr & 0x003FF000) >> 12;

  page_tables[page_dir_index].entries[page_table_index] = 0;
  invlpg(virtual_addr);
}
