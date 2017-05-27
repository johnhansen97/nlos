#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_SCAN_QUEUE_SIZE 32
#define KEYBOARD_CHAR_QUEUE_SIZE 32

void keyboard_init(void);
void keyboard_update(void);
char get_ch(void);
#endif
