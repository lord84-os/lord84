#include <stdint.h>
void kkill(void);

typedef char link_symbol_ptr[];

#define ALIGN_UP(x, base) (((x) + (base) - 1) & ~((base) - 1))
#define ALIGN_DOWN(x, base) ((x) & ~((base) - 1))

void *kmalloc(uint64_t size);