#include "queue.h"

void queue_init(queue_t *q,
		uint32_t *data,
		uint32_t cap) {
  q->size = 0;
  q->head = 0;
  q->tail = 0;
  q->array = data;
  q->capacity = cap;
}

void enqueue(queue_t *q,
	     uint32_t value) {
  if (q->size == q->capacity)
    return;

  q->array[q->tail] = value;
  q->tail = (q->tail + 1) % q->capacity;
  q->size++;
}

uint32_t dequeue(queue_t *q) {
  uint32_t ret;
  if (q->size == 0)
    return 0;

  ret = q->array[q->head];
  q->head = (q->head + 1) % q->capacity;
  q->size--;
  return ret;
}
