
#include <stdint.h>
#include <stdio.h>
#include <lord84.h>
#include "../sys/acpi.h"

#define IOREGSEL    0x0
#define IOWIN       0x10

#define IOAPICID    0
#define IOAPICVER   1
#define IOAPICARB   2
#define IOREDTBL(x) (0x10 + (x * 2)) // 0-23 registers

extern uint64_t hhdmoffset;

uint64_t ioapic_address;

void ioapic_write_reg(uint8_t reg, uint32_t data){
    /* First we load IOREGSEL with the register we want to access */
    *(uint64_t*)(ioapic_address + IOREGSEL) = reg;

    /* Then we write the data to the IOWIN register */
    *(uint64_t*)(ioapic_address + IOWIN) = data;
}

uint32_t ioapic_read_reg(uint8_t reg){
    *(uint64_t*)(ioapic_address + IOREGSEL) = reg;

    return *(uint64_t*)(ioapic_address + IOWIN);
}

void write_redir_entry(uint8_t reg, uint64_t data){
    /* First write lower 32-bits of the data to the specified IOREDTBL register */
    ioapic_write_reg(IOREDTBL(reg), data & 0xFFFFFFFF);
    /* Then write the upper 32-bits */
    ioapic_write_reg(IOREDTBL(reg+1), data >> 32); /* fix */
}

extern madt_t *madt;

void ioapic_init(void){

    ioapic_t *ioapic = (ioapic_t*) find_ics(0x1);
    
    if(!ioapic){
        klog(LOG_ERROR, __func__, "IOAPIC ICS not found\n");
        kkill();
    }

    ioapic_address = ioapic->ioapic_address + hhdmoffset;

    




}