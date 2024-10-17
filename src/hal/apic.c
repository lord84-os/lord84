#include "../sys/acpi.h"
#include "timer.h"
#include "ioapic.h"
#include <stdio.h>
#include <lord84.h>
#include <cpuid.h> // GCC specific

#define LAPIC_ID_REG            0x020
#define LAPIC_EOI_REG           0x0B0
#define LAPIC_SPURIOUS_REG      0x0F0
#define LAPIC_ERR_REG           0x280

#define LAPIC_LINT0_REG         0x350
#define LAPIC_LINT1_REG         0x360

#define LAPIC_INITIAL_CNT_REG   0x380
#define LAPIC_CURRENT_CNT_REG   0x390
#define LAPIC_DIVIDE_CONF_REG   0x3E0

extern madt_t *madt;
extern uint64_t hhdmoffset;

uint64_t lapic_address;

void lapic_write_reg(uint32_t reg, uint32_t data){
    *((uint32_t*)(lapic_address+reg)) = data;
}

uint32_t lapic_read_reg(uint32_t reg){
    return(*((uint32_t*)(lapic_address+reg)));
}

void apic_init(void){
    lapic_address = madt->lic_address + hhdmoffset;

    lapic_ao_t *lapic_ao = (lapic_ao_t*) find_ics(0x5); // Local APIC Address Override

    if(lapic_ao){
        lapic_address = lapic_ao->lapic_address + hhdmoffset;
    }
    
    /* Enable the lapic and set the spurious interrupt vector to 0xFF */
    lapic_write_reg(LAPIC_SPURIOUS_REG, 0x1FF);

    klog(LOG_INFO, __func__, "Initializing IOAPIC");
    ioapic_init();

    timer_init();

}