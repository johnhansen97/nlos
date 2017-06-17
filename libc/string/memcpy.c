#include <string.h>

void *memcpy(void *s, void *ct, size_t n) {
  char *dest = (char *)s;
  for (; n > 0; n--) {
    *dest = *(char *)ct;
    dest++;
    ct++;
  }
  return s;
}
