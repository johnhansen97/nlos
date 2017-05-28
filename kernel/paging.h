#ifndef PAGING_H
#define PAGING_H

#define PAGE_DIR_ENTRY_ADDR   0xFFFFF000
#define PAGE_DIR_ENTRY_AVAIL  0x00000E00
#define PAGE_DIR_ENTRY_IGNORE 0x00000100
#define PAGE_DIR_ENTRY_SIZE   0x00000080
#define PAGE_DIR_ENTRY_ACCESS 0x00000020
#define PAGE_DIR_ENTRY_CACHE  0x00000010
#define PAGE_DIR_ENTRY_WRITE  0x00000008
#define PAGE_DIR_ENTRY_USER   0x00000004
#define PAGE_DIR_ENTRY_RW     0x00000002
#define PAGE_DIR_ENTRY_PRES   0x00000001

#define PAGE_TABLE_ENTRY_ADDR   0xFFFFF000
#define PAGE_TABLE_ENTRY_AVAIL  0x00000E00
#define PAGE_TABLE_ENTRY_GLOBAL 0x00000100
#define PAGE_TABLE_ENTRY_DIRTY  0x00000040
#define PAGE_TABLE_ENTRY_ACCESS 0x00000020
#define PAGE_TABLE_ENTRY_CACHE  0x00000010
#define PAGE_TABLE_ENTRY_WRITE  0x00000008
#define PAGE_TABLE_ENTRY_USER   0x00000004
#define PAGE_TABLE_ENTRY_RW     0x00000002
#define PAGE_TABLE_ENTRY_PRES   0x00000001

void init_paging(void);
void page_map(uintptr_t virtual_addr, uintptr_t physical_addr);
void insert_addr_block(uintptr_t start, uint32_t size);
uintptr_t addr_block_remove(uint32_t size);
void print_addr_blocks(void);
uintptr_t page_frame_alloc(uint32_t n);
void page_frame_free(uintptr_t start, uint32_t size);
uintptr_t palloc(uint32_t n);
#endif
