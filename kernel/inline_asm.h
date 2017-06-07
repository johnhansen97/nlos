#ifndef INLINE_ASM_H
#define INLINE_ASM_H

static inline void outb(uint16_t port, uint8_t val) {
  asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile ( "inb %1, %0"
		 : "=a"(ret)
		 : "Nd"(port) );
  return ret;
}

static inline void io_wait(void) {
  asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}

static inline void hlt(void) {
  asm ( "hlt" );
}

static inline void invlpg(uintptr_t m) {
  asm volatile ("invlpg (%0)"
		:
		:"b"(m)
		: "memory");
}

static inline uint32_t read_cr3(void) {
  uint32_t ret;
  asm volatile ("mov %%cr3, %0"
		: "=r"(ret));
  return ret;
}

static inline void write_cr3(uint32_t val) {
  asm volatile ("mov %0, %%cr3"
		:
		: "r"(val));
}
#endif
