#include <stdint.h>
#include "idt.h"
void isr08_wrapper(void);
void isr09_wrapper(void);
void isr0E(void);
idt_t idt;

void idt_init(idt_t *t) {
  int i;
  for (i = 0; i < 256; i++) {
    idt_desc_init(&(t->entries[i]));
  }

  idt_desc_new(&idt.entries[0x08], (uint32_t) &isr08_wrapper, 0x10);
  idt_desc_new(&idt.entries[0x09], (uint32_t) &isr09_wrapper, 0x10);
  idt_desc_new(&idt.entries[0x0E], (uint32_t) &isr0E, 0x10);
  return;
}

void idt_desc_init(idt_desc_t *t) {
  t->offset_1 = 0;
  t->selector = 2;
  t->zero = 0;
  t->type_attr = 0;
  t->offset_2 = 0;
}
  
void idt_desc_new(idt_desc_t *t, uint32_t offset, uint16_t selector) {
  t->selector = selector;
  t->offset_1 = offset & 0xffff;
  t->offset_2 = offset >> 16;
  t->zero = 0;
  t->type_attr = 0x8e;
}
