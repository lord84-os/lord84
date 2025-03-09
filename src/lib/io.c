#include <stdint.h>

uint64_t rdmsr(uint64_t msr){
	uint32_t low, high;
	asm volatile (
		"rdmsr"
		: "=a"(low), "=d"(high)
		: "c"(msr)
	);
	return ((uint64_t)high << 32) | low;
}

void wrmsr(uint64_t msr, uint64_t value){
	uint32_t low = value & 0xFFFFFFFF;
	uint32_t high = value >> 32;
	asm volatile (
		"wrmsr"
		:
		: "c"(msr), "a"(low), "d"(high)
	);
}

void outb(uint16_t port, uint8_t val){
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

void outw(uint16_t port, uint16_t val){
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

void outl(uint16_t port, uint32_t val){
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) : "memory");
}


uint8_t inb(uint16_t port){
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

uint16_t inw(uint16_t port){
    uint16_t ret;
    asm volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}


uint32_t inl(uint16_t port){
    uint32_t ret;
    asm volatile ( "inl %1, %0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

