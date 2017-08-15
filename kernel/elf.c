#include <stdint.h>
#include <string.h>
#include "elf.h"
#include "paging.h"
#include "process.h"
#include "terminal.h"
#include "inline_asm.h"

extern terminal_t std;

/**
 * Load an executable from an elf file located in physical memory
 * @param pmem_start physical address of the elf file
 * @param pmem_end physical address immediately after the file
 * @param p the process in which the elf will be loaded
 * @return 0 on success, non-zero on error
 */
int loadElf(uintptr_t pmem_start, uintptr_t pmem_end, process_t *p) {
  int num_pages_file = (pmem_end - pmem_start) >> 12;
  int offset, i;
  elf_header_t *vmem_start;

  //calculate how many pages are needed for the file
  if ((pmem_end - pmem_start) & 0x0FFF) {
    num_pages_file++;
  }

  //allocate virtual address space
  vmem_start = (elf_header_t *) addr_block_remove(num_pages_file);

  //map the elf file into kernel memory
  for (offset = 0; offset < num_pages_file; offset++) {
    page_map((uintptr_t) vmem_start + (offset << 12), pmem_start + (offset << 12));
  }

  //make sure the executable is valid
  if (!elfIsValidExec(vmem_start)) {
    print_str(&std, "loadElf: elf is not a valid executable, aborting.\n");

    for (offset = 0; (offset << 12) < num_pages_file; offset += 0x1000) {
      page_unmap((uintptr_t) vmem_start + offset);
    }

    insert_addr_block((uintptr_t) vmem_start, num_pages_file);
    
    return 1;
  }

  //switch to process address space
  write_cr3(p->page_dir_physical);

  //load sections
  for (i = 0; i < vmem_start->program_entries; i++) {
    elfLoadSection((elf_program_t *)((void *)vmem_start + vmem_start->program_header) + i, p,
		   vmem_start);
  }

  //init stack
  process_init_stack(p, vmem_start->entry);

  //free up memory
  for (offset = 0; (offset << 12) < num_pages_file; offset += 0x1000) {
    page_unmap((uintptr_t) vmem_start + offset);
  }

  insert_addr_block((uintptr_t) vmem_start, num_pages_file);
  return 0;
}

/**
 * Check if a given elf file can be loaded and executed by NLOS.
 * @param file pointer to an elf file in memory
 * @return 1 if valid, 0 otherwise
 */
int elfIsValidExec(elf_header_t *file) {
  int i;
  elf_program_t *program_headers;

  //verify that file is elf
  if (file->elf_magic[0] != ELF_MAGIC_0 ||
      file->elf_magic[1] != ELF_MAGIC_1 ||
      file->elf_magic[2] != ELF_MAGIC_2 ||
      file->elf_magic[3] != ELF_MAGIC_3) {
    print_str(&std, "elfIsValidExec: elf magic field is invalid.\n");
    return 0;
  }

  //verify elf 32bit format
  if (file->bits != ELF_BITS_32) {
    print_str(&std, "elfIsValidExec: not 32 bits.\n");
    return 0;
  }

  //verify little endianness
  if (file->endianness != ELF_ENDIAN_LITTLE) {
    print_str(&std, "elfIsValidExec: not little endian.\n");
    return 0;
  }

  //verify abi
  if (file->abi != ELF_ABI_SYSV) {
    print_str(&std, "elfIsValidExec: ABI is not SystemV.\n");
    return 0;
  }

  //verify type
  if (file->type != ELF_TYPE_EXEC) {
    print_str(&std, "elfIsValidExec: not of the type 'executable'.\n");
    return 0;
  }

  //verify instruction set
  if (file->instr_set != ELF_INSTR_SET_X86) {
    print_str(&std, "elfIsValidExec: instruction set is not x86.\n");
    return 0;
  }

  //verify entry
  if (file->entry == 0 || file->entry >= 0xC0000000) {
    print_str(&std, "elfIsValidExec: entry point is null or in kernel space.\n");
    return 0;
  }

  //verify elf header size
  if (file->header_size != sizeof(elf_header_t)) {
    print_str(&std, "elfIsValidExec: elf header size mismatch.\n");
    return 0;
  }

  //verify program entry size
  if (file->program_entry_size != sizeof(elf_program_t)) {
    print_str(&std, "elfIsValidExec: elf program header entry size mismatch.\n");
    return 0;
  }
  
  program_headers = (elf_program_t *)((uintptr_t)file + file->program_header);
  
  for (i = 0; i < file->program_entries; i++) {
    if (program_headers[i].segment_type > 1) {
      print_str(&std, "elfIsValidExec: invalid type for program header ");
      print_hex(&std, i);
      print_str(&std, ".\n");
      return 0;
    }

    if ((program_headers[i].align - 1) & program_headers[i].align) {
      print_str(&std, "elfIsValidExec: align attribute for program header ");
      print_hex(&std, i);
      print_str(&std, " is not a power of 2.\n");
      return 0;
    }
  }

  return 1;
}

void elfLoadSection(elf_program_t *section, process_t *p, elf_header_t *file) {
  uintptr_t vaddr;
  uintptr_t vaddr_end;

  //ignore if section type is null
  if (section->segment_type == ELF_P_TYPE_NULL) {
    return;
  }

  //allocate memory
  vaddr = section->vaddr;
  vaddr_end = vaddr + section->mem_size;
  if (vaddr_end & 0xFFF) {
    vaddr_end = (vaddr_end + 0x1000) & 0xFFFFF000;
  }
  for (; vaddr < vaddr_end; vaddr += 0x1000) {
    process_map_page(p, vaddr, page_frame_alloc(1));
    zeroPage(vaddr);
  }

  memcpy((void *)section->vaddr, (void *)file + section->offset, section->file_size);
  return;
}
