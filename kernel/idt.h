#ifndef IDT_H
#define IDT_H

typedef struct idt_desc {
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
} idt_desc_t;

typedef struct idt {
  idt_desc_t entries[256];
} idt_t;

void idt_init(idt_t *t);
void idt_desc_init(idt_desc_t *t);
void idt_desc_new(idt_desc_t *t, uint32_t offset, uint16_t selector);
#endif
