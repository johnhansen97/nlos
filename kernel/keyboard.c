#include "keyboard.h"
#include "queue.h"
#include "inline_asm.h"
#include "kernel.h"

static uint32_t scan_queue_array[KEYBOARD_SCAN_QUEUE_SIZE];
static uint32_t char_queue_array[KEYBOARD_CHAR_QUEUE_SIZE];

queue_t scan_queue;
queue_t char_queue;

static uint8_t scancode_byte_n = 0;
static uint8_t caps_lock = 0;

static enum key_state {
  KEY_EMPTY,
  KEY_UP,
  KEY_DOWN,
} key_states[256];

typedef enum {
  EMPTY,
  NON_ASCII,
  ASCII_CTRL,
  ASCII_SYMB,
  ASCII_LETTER,
  ASCII_NUMBER,
  RELEASE,
  MORE_BYTES
} scancode_type_t;

//scancode set 1, first byte
scancode_type_t scancode_set_1_1_type[256] = {
  //0x00
  EMPTY, ASCII_CTRL, ASCII_NUMBER, ASCII_NUMBER,
  ASCII_NUMBER, ASCII_NUMBER, ASCII_NUMBER, ASCII_NUMBER,
  ASCII_NUMBER, ASCII_NUMBER, ASCII_NUMBER, ASCII_NUMBER,
  ASCII_SYMB, ASCII_SYMB, ASCII_CTRL, ASCII_CTRL,
  //0x10
  ASCII_LETTER, ASCII_LETTER, ASCII_LETTER, ASCII_LETTER,
  ASCII_LETTER, ASCII_LETTER, ASCII_LETTER, ASCII_LETTER,
  ASCII_LETTER, ASCII_LETTER, ASCII_SYMB, ASCII_SYMB,
  ASCII_CTRL, NON_ASCII, ASCII_LETTER, ASCII_LETTER,
  //0x20
  ASCII_LETTER, ASCII_LETTER, ASCII_LETTER, ASCII_LETTER,
  ASCII_LETTER, ASCII_LETTER, ASCII_LETTER, ASCII_SYMB,
  ASCII_SYMB, ASCII_SYMB, NON_ASCII, ASCII_SYMB,
  ASCII_LETTER, ASCII_LETTER, ASCII_LETTER, ASCII_LETTER,
  //0x30
  ASCII_LETTER, ASCII_LETTER, ASCII_LETTER, ASCII_SYMB,
  ASCII_SYMB, ASCII_SYMB, NON_ASCII, ASCII_SYMB,
  NON_ASCII, ASCII_SYMB, NON_ASCII, NON_ASCII,
  NON_ASCII, NON_ASCII, NON_ASCII, NON_ASCII,
  //0x40
  NON_ASCII, NON_ASCII, NON_ASCII, NON_ASCII,
  NON_ASCII, NON_ASCII, NON_ASCII, ASCII_NUMBER,
  ASCII_NUMBER, ASCII_NUMBER, ASCII_SYMB, ASCII_NUMBER,
  ASCII_NUMBER, ASCII_NUMBER, ASCII_SYMB, ASCII_NUMBER,
  //0x50
  ASCII_NUMBER, ASCII_NUMBER, ASCII_NUMBER, ASCII_SYMB,
  EMPTY, EMPTY, EMPTY, NON_ASCII,
  NON_ASCII, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY,
  //0x60
  EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY,
  //0x70
  EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY,
  //0x80
  EMPTY, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  //0x90
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  //0xA0
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  //0xB0
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  //0xC0
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  RELEASE, RELEASE, RELEASE, RELEASE,
  //0xD0
  RELEASE, RELEASE, RELEASE, RELEASE,
  EMPTY, EMPTY, EMPTY, RELEASE,
  RELEASE, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY,
  //0xE0
  MORE_BYTES
};

uint8_t scancode_set_1_ascii_l[256] = {
  [0x01] = '\e', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', '0', '-', '=', '\b',
  [0x0f] = '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u',
  'i', 'o', 'p', '[', ']', '\n', 
  [0x1e] = 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k',
  'l', ';', '\'', '`',
  [0x2b] = '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
  ',', '.', '/', 
  [0x37] = '*',
  [0x39] = ' ',
  [0x47] = '7', '8', '9', '-', '4', '5', '6', '+',
  '1', '2', '3', '0', '.'
};

uint8_t scancode_set_1_ascii_u[256] = {
  [0x01] = '\e', '!', '@', '#', '$', '%', '^', '&',
  '*', '(', ')', '_', '+', '\b',
  [0x0f] = '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
  'I', 'O', 'P', '{', '}', '\n', 
  [0x1e] = 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K',
  'L', ':', '\"', '~',
  [0x2b] = '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M',
  '<', '>', '?',
  [0x37] = '*',
  [0x39] = ' ',
  [0x47] = '7', '8', '9', '-', '4', '5', '6', '+',
  '1', '2', '3', '0', '.'
};

void keyboard_init(void) {
  queue_init(&scan_queue, scan_queue_array, KEYBOARD_SCAN_QUEUE_SIZE);
  queue_init(&char_queue, char_queue_array, KEYBOARD_CHAR_QUEUE_SIZE);
}

void keyboard_update(void) {
  uint8_t scan;
  scancode_type_t type;

  while (scan_queue.size != 0) {
    scan = dequeue(&scan_queue);
    type = scancode_set_1_1_type[scan];
    
    switch (type) {
    case MORE_BYTES:
      scancode_byte_n++;
      continue;
      break;

    case RELEASE:
      key_states[scan - 0x80] = KEY_UP;
      scancode_byte_n = 0;
      continue;
      break;

    case NON_ASCII:
      key_states[scan] = KEY_DOWN;
      scancode_byte_n = 0;
      /* if scan is caps lock key */
      if (scan == 0x3a) {
	caps_lock = ~caps_lock;
      }
      continue;
      break;

    case ASCII_SYMB:
    case ASCII_NUMBER:
    case ASCII_CTRL:
      /* if either shift key is down */
      if (key_states[0x2a] == KEY_DOWN || key_states[0x36] == KEY_DOWN) {
	enqueue(&char_queue, scancode_set_1_ascii_u[scan]);
      } else {
	enqueue(&char_queue, scancode_set_1_ascii_l[scan]);
      }
      scancode_byte_n = 0;
      continue;
      break;

    case ASCII_LETTER:
      /* if either shift key is pressed */
      if (key_states[0x2a] == KEY_DOWN || key_states[0x36] == KEY_DOWN) {
	if (caps_lock) {
	  enqueue(&char_queue, scancode_set_1_ascii_l[scan]);
	} else {
	  enqueue(&char_queue, scancode_set_1_ascii_u[scan]);
	}
      } else {
	if (caps_lock) {
	  enqueue(&char_queue, scancode_set_1_ascii_u[scan]);
	} else {
	  enqueue(&char_queue, scancode_set_1_ascii_l[scan]);
	}
      }
      scancode_byte_n = 0;
      continue;
      break;

    case EMPTY:
    default:
      kernel_panic_message("Keyboard error");
      continue;
      break;
    }
    
  }
}

char get_ch(void) {
  while (char_queue.size == 0) {
    hlt();
    keyboard_update();
  }
  return (char) dequeue(&char_queue);
}
