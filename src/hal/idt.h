#include <stdbool.h>
#include <stdint.h>

typedef struct idt_descriptor {
    uint16_t offset_low;
    uint16_t segment_sel;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_high;
    uint32_t offset_higher;
    uint32_t reserved;
} __attribute((packed))idt_descriptor;

typedef struct idt_register {
    uint16_t limit;
    uint64_t base_address;
} __attribute((packed)) idt_register;

typedef struct interrupt_frame {
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx, rbx, rax;
	uint64_t int_no, err;
	uint64_t cs, rip, rsp, ss, rflags;
} __attribute((packed)) interrupt_frame;

typedef struct irq_t {
    void *base;
    bool in_use;
}irq_t;

void set_idt_descriptor(uint8_t vector, void *base, uint8_t flags);

void set_idt(void);