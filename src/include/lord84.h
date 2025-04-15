#include <stdint.h>
void kkill(void);

typedef char link_symbol_ptr[];

#define ALIGN_UP_MASK(x, mask)   (((x) + (mask)) & ~(mask))
#define ALIGN_UP(x, val)         ALIGN_UP_MASK(x, (typeof(x))(val) - 1)

#define ALIGN_DOWN_MASK(x, mask) ((x) & ~(mask))
#define ALIGN_DOWN(x, val)       ALIGN_DOWN_MASK(x, (typeof(x))(val) - 1)

#define IS_ALIGNED_MASK(x, mask) (((x) & (mask)) == 0)
#define IS_ALIGNED(x, val)       IS_ALIGNED_MASK(x, (typeof(x))(val) - 1)

#define PAGE_ROUND_UP(size)   ALIGN_UP(size, PAGE_SIZE)
#define PAGE_ROUND_DOWN(size) ALIGN_DOWN(size, PAGE_SIZE)

void *kmalloc(uint64_t size);

void kpanic(const char *reason);

typedef struct stack_frame {
	uint64_t cr2, r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx, rbx, rax;
	uint64_t rip, cs, rsp, ss;
} __attribute((packed)) stack_frame;