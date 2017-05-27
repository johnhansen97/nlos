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

static inline void hlt(void) {
  asm ( "hlt" );
}

static inline void invlpg(uintptr_t m) {
  asm volatile ("invlpg (%0)"
		:
		:"b"(m)
		: "memory");
}
#endif
