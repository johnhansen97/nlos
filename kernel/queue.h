#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

typedef struct queue {
  uint32_t head, tail;
  uint32_t capacity;
  uint32_t size;
  uint32_t *array;
} queue_t;

void queue_init(queue_t *q,
		uint32_t *data,
		uint32_t cap);
void enqueue(queue_t *q,
	     uint32_t value);
uint32_t dequeue(queue_t *q);

#endif
