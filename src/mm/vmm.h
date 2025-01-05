
#include <stdint.h>

#define PTE_BIT_PRESENT 0x1                 // Present bit
#define PTE_BIT_RW      0x2                 // Read/write bit
#define PTE_BIT_US      0x4                 // User and Supervisor bit
#define PTE_BIT_NX      0x4000000000000000  // Non-executable bit

#define PAGE_SIZE 4096

void tlb_flush(void);
void vmm_map_page(uint64_t *page_map, uint64_t virt_address, uint64_t phys_address, uint64_t flags);
int vmm_map_continous_pages(uint64_t virt_addr, uint64_t size, uint64_t flags);
void vmm_free_page(uint64_t *page_map, uint64_t virt_addr);
void vmm_init();
void vmm_set_ctx(uint64_t *page_map);
void *kernel_allocate_memory(uint64_t size, uint64_t flags);

typedef char link_symbol_ptr[];

