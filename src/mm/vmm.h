
#include <stdint.h>

#define PTE_BIT_PRESENT 0x1                 // Present bit
#define PTE_BIT_RW      0x2                 // Read/write bit
#define PTE_BIT_US      0x4                 // User and Supervisor bit
#define PTE_BIT_NX      0x4000000000000000  // Non-executable bit

void tlb_flush(void);
void vmm_map_page(uint64_t *page_map, uint64_t *virt_address, uint64_t* phys_address, uint64_t flags);
void vmm_init();
void vmm_set_ctx(uint64_t*page_map);

#define ALIGN_UP(x, base) (((x) + (base) - 1) & ~((base) - 1))
#define ALIGN_DOWN(x, base) ((x) & ~((base) - 1))

typedef char link_symbol_ptr[];