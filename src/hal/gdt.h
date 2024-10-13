#include <stdint.h>

typedef struct gdt_descriptor {
    uint16_t limit_low;
    uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute((packed)) gdt_descriptor;

typedef struct gdt_register {
    uint16_t limit;
    uint64_t base_address;
} __attribute((packed)) gdt_register;

void set_gdt(void);