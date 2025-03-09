#include <stdint.h>
void kernel_heap_init();

void heap_free(uint64_t *addr);
uint64_t *heap_alloc();

void *kmalloc(uint64_t size);
void kfree(void *addr);

#define KERNEL_HEAP_SIZE    0x10000000