#include <stdint.h>
#include "paging.h"

int liballoc_lock() {
  asm ("cli");
  return 0;
}

int liballoc_unlock() {
  asm ("sti");
  return 0;
}

void* liballoc_alloc(int n) {
  return (void *) palloc(n);
}

int liballoc_free(void* addr, int n) {
  pfree((uintptr_t)addr);
  return n - n;
}
