#include "gdt.h"
#include <stdio.h>

gdt_descriptor gdt[5] = {0};

gdt_register gdtr = {sizeof(gdt), (uint64_t)((uint64_t*)gdt)};

extern void s_load_gdt();

void gdt_set_entry(int num, unsigned long long base, unsigned long long limit, unsigned char access, unsigned char granularity){
    // descriptor base access
    gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	// descriptor limits
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	// granularity and access flag
	gdt[num].granularity |= (granularity & 0xF) << 4;
	gdt[num].access = access;
}

void set_gdt(void){
    kprintf("gdt1");
    gdt_set_entry(0, 0, 0, 0, 0);               // null segment     offset 0x00
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xA);    // kernel code      offset 0x08
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC);    // kernel data      offset 0x10
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xA);    // userspace code   offset 0x18
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xC);    // userspace data   offset 0x20
    kprintf("gdt2");
    s_load_gdt();
    kprintf("gdt3");

}