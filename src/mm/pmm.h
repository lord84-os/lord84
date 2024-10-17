#include <stdbool.h>
#include <stdint.h>

#define BLOCK_SIZE 4096

typedef struct free_page_t {
    struct free_page_t *next;
    uint8_t _padding[4088];
} __attribute((packed)) free_page_t;

void pmm_init(void);
uint64_t *pmm_alloc();
void pmm_free(uint64_t *addr);

