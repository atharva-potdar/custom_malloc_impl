#include <unistd.h>

void* my_malloc(size_t size) {
  return sbrk(size);
}

void my_free(void* ptr);

int main(void) {
  char *p = my_malloc(32);
  // my_free(p);
  return 0;
}
