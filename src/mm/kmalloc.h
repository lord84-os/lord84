#include <stdint.h>
void kernel_heap_init();

void heap_free(uint64_t *addr);
uint64_t *heap_alloc();

void *kmalloc(uint64_t size);