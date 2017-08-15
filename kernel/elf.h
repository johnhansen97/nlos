#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#define ELF_MAGIC_0 0x7F
#define ELF_MAGIC_1 'E'
#define ELF_MAGIC_2 'L'
#define ELF_MAGIC_3 'F'

#define ELF_BITS_32 1
#define ELF_BITS_64 2

#define ELF_ENDIAN_LITTLE 1
#define ELF_ENDIAN_BIG    2

#define ELF_ABI_SYSV 0

#define ELF_TYPE_RELOC 1
#define ELF_TYPE_EXEC  2
#define ELF_TYPE_SHARE 3
#define ELF_TYPE_CORE  4

#define ELF_INSTR_SET_X86 3
//TODO: add other architectures

#define ELF_P_TYPE_NULL    0
#define ELF_P_TYPE_LOAD    1
#define ELF_P_TYPE_DYNAMIC 2
#define ELF_P_TYPE_INTERP  3
#define ELF_P_TYPE_NOTE    4

#define ELF_P_FLAG_EXEC  1
#define ELF_P_FLAG_WRITE 2
#define ELF_P_FLAG_READ  4

typedef struct {
  uint8_t elf_magic[4];          //0
  uint8_t bits;                  //4
  uint8_t endianness;            //5
  uint8_t elf_version;           //6
  uint8_t abi;                   //7
  uint32_t unused[2];            //8
  uint16_t type;                 //16
  uint16_t instr_set;            //18
  uint32_t file_version;         //20
  uint32_t entry;                //24
  uint32_t program_header;       //28
  uint32_t seciton_header;       //32
  uint32_t flags;                //36
  uint16_t header_size;          //40
  uint16_t program_entry_size;   //42
  uint16_t program_entries;      //44
  uint16_t section_entry_size;   //46
  uint16_t section_entries;      //48
  uint16_t shstrindex;           //50
} elf_header_t;

typedef struct {
  uint32_t segment_type;     //0
  uint32_t offset;           //4
  uint32_t vaddr;            //8
  uint32_t undefined;        //12
  uint32_t file_size;        //16
  uint32_t mem_size;         //20
  uint32_t flags;            //24
  uint32_t align;            //28
} elf_program_t;

#include "process.h"

int loadElf(uintptr_t pmem_start, uintptr_t pmem_end, process_t *p);
int elfIsValidExec(elf_header_t *file);
void elfLoadSection(elf_program_t *section, process_t *p, elf_header_t *file);



#endif
