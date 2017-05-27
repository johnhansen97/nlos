#include <stdint.h>
#include "terminal.h"
#include "inline_asm.h"
#include "kernel.h"

void terminal_mv_cursor(terminal_t *t) {
  uint16_t position = t->row * t->width + t->column;

  outb(VGA_PORT_CTRL, 0x0F);
  outb(VGA_PORT_DATA, (uint8_t)(position & 0xFF));

  outb(VGA_PORT_CTRL, 0x0E);
  outb(VGA_PORT_DATA, (uint8_t)((position >> 8) & 0xFF));
}

void terminal_init(terminal_t *t) {
  uint8_t *p;
  t->row = t->column = 0;
  t->width = TERMINAL_STD_WIDTH;
  t->height = TERMINAL_STD_HEIGHT;
  t->addr = (uint8_t *) TERMINAL_STD_ADDR;
  terminal_set_color(t, VGA_COLOR_WHITE, VGA_COLOR_BLACK);

  /* clear screen */
  p = t->addr;
  while (p < t->width * t->height * 2 + t->addr) {
    *p = ' ';
    p++;
    *p = t->color;
    p++;
  }
}

void terminal_set_color(terminal_t *t, enum vga_color fg, enum vga_color bg) {
  t->color = fg | bg << 4;
}

static void terminal_nl(terminal_t *t) {
  uint8_t *a, *b;

  t->row++;
  t->column = 0;
  if (t->row == t->height) {
    a = (uint8_t *) t->addr;
    b = (uint8_t *) (a + t->width * 2);
    while (b < (uint8_t *) t->addr + t->width * t->height * 2) {
      *a = *b;
      if ((int)b % 2) {
	*b = 0x07;
      } else {
	*b = ' ';
      }
      a++;
      b++;
    }
    t->row--;
  }
}

void print_ch(terminal_t *t, char c) {
  uint8_t *p;

  if (t->column == t->width) {
    terminal_nl(t);
  }

  switch (c) {
  case '\n':
    terminal_nl(t);
    break;

  case '\b':
    if (t->column) {
      t->column--;
    } else {
      if (t->row) {
	t->row--;
	t->column = t->width - 1;
      } else {
	t->row = 0;
	t->column = 0;
      }
    }
    p = t->addr + (t->row * t->width + t->column) * 2;
    *p = ' ';
    break;

  case '\t':
    t->column = ((t->column / 4) + 1) * 4;
    break;

  case '\e':
    kernel_panic_message("<ESC> Key Pressed");
    break;

  default:
    p = t->addr + (t->row * t->width + t->column) * 2;
    *p = c;
    p++;
    *p = t->color;
    t->column++;
  }
}

void print_str(terminal_t *t, const char *s) {
  while (*s) {
    print_ch(t, *s);
    s++;
  }
  terminal_mv_cursor(t);
}

void print_hex(terminal_t *t, uint32_t n) {
  char s[11];
  int i;
  s[0] = '0';
  s[1] = 'x';
  s[10] = 0;

  for (i = 9; i > 1; i--) {
    s[i] = n & 0xF;
    n = n >> 4;
    if (s[i] < 10) {
      s[i] = s[i] + '0';
    } else {
      s[i] = s[i] + 'A' - 10;
    }
  }

  print_str(t, s);

}
