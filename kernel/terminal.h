#ifndef TERMINAL_H
#define TERMINAL_H

#define TERMINAL_STD_WIDTH   80
#define TERMINAL_STD_HEIGHT  25
#define TERMINAL_STD_ADDR    0xC00B8000

#define VGA_PORT_CTRL        0x3D4
#define VGA_PORT_DATA        0x3D5

enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

typedef struct terminal {
  uint8_t row, column;
  uint8_t width, height;
  uint8_t *addr;
  uint8_t color;
} terminal_t;

void terminal_init(terminal_t *t);
void terminal_set_color(terminal_t *t, enum vga_color fg, enum vga_color bg);
void terminal_mv_cursor(terminal_t *t);
void print_ch(terminal_t *t, char c);
void print_str(terminal_t *t, const char *s);
void print_hex(terminal_t *t, uint32_t n);
#endif
