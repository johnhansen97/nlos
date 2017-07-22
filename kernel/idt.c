#include <stdint.h>
#include "idt.h"
#include "inline_asm.h"
void isr20_wrapper(void);
void isr21_wrapper(void);
void isr0E(void);
void isr80(void);
idt_t idt;

/**
 * Map IRQs to interrupts 0x20-0x2f.
 */
static void remap_pic(void) {
  uint8_t mask1, mask2;

  mask1 = inb(PIC1_DATA);
  mask2 = inb(PIC2_DATA);

  //Begin initialization sequence
  outb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
  io_wait();

  //Set vector offset for master and slave PICs
  outb(PIC1_DATA, 0x20);
  io_wait();
  outb(PIC2_DATA, 0x28);
  io_wait();

  //Configure master/slave identity
  outb(PIC1_DATA, 4);
  io_wait();
  outb(PIC2_DATA, 2);
  io_wait();
  
  //???
  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  //restore masks
  outb(PIC1_DATA, mask1);
  outb(PIC2_DATA, mask2);
  
}

void idt_init(idt_t *t) {
  int i;
  for (i = 0; i < 256; i++) {
    idt_desc_init(&(t->entries[i]));
  }

  remap_pic();

  idt_desc_new(&idt.entries[0x20], (uint32_t) &isr20_wrapper, 0x10, 0x8e);
  idt_desc_new(&idt.entries[0x21], (uint32_t) &isr21_wrapper, 0x10, 0x8e);
  idt_desc_new(&idt.entries[0x0E], (uint32_t) &isr0E, 0x10, 0x8e);
  idt_desc_new(&idt.entries[0x80], (uint32_t) &isr80, 0x10, 0xee);
  return;
}

void idt_desc_init(idt_desc_t *t) {
  t->offset_1 = 0;
  t->selector = 0x10;
  t->zero = 0;
  t->type_attr = 0;
  t->offset_2 = 0;
}
  
void idt_desc_new(idt_desc_t *t, uint32_t offset, uint16_t selector, uint8_t type_attr) {
  t->selector = selector;
  t->offset_1 = offset & 0xffff;
  t->offset_2 = offset >> 16;
  t->zero = 0;
  t->type_attr = type_attr;
}
