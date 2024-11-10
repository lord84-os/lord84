#include "../sys/acpi.h"
#include "../drivers/pmt.h"
#include "timer.h"
#include "ioapic.h"
#include <stdio.h>
#include <lord84.h>
#include <cpuid.h> // GCC specific

#define LAPIC_ID_REG                0x020
#define LAPIC_EOI_REG               0x0B0
#define LAPIC_SPURIOUS_REG          0x0F0
#define LAPIC_ERR_REG               0x280

#define LAPIC_LINT0_REG             0x350
#define LAPIC_LINT1_REG             0x360

#define LAPIC_LVT_TIMER_REG         0x320
#define LAPIC_TIMER_INITIAL_CNT_REG 0x380
#define LAPIC_TIMER_CURRENT_CNT_REG 0x390
#define LAPIC_TIMER_DIVIDER_REG     0x3E0

#define LAPIC_TIMER_MASK            (1 << 16)
#define LAPIC_TIMER_UNMASK          0xFFFEFFFF

#define LAPIC_TIMER_PERIODIC        (1 << 17)
#define LAPIC_TIMER_VECTOR          69


extern madt_t *madt;
extern uint64_t hhdmoffset;

uint64_t lapic_timer_ticks = 0;

uint64_t lapic_address = 0;
uint64_t timer_speed_us = 0;

void lapic_write_reg(uint32_t reg, uint32_t data){
    *((uint32_t*)(lapic_address+reg)) = data;
}

uint32_t lapic_read_reg(uint32_t reg){
    return(*((uint32_t*)(lapic_address+reg)));
}

void apic_sleep(uint64_t ms){
    int curcnt = lapic_timer_ticks;
    while (lapic_timer_ticks - curcnt < ms) {
        asm("nop");
    }
}

void lapic_timer_init(){
    /* Stop the APIC timer */
    lapic_write_reg(LAPIC_TIMER_INITIAL_CNT_REG, 0);

    /* Set the divisor to 16 */
    lapic_write_reg(LAPIC_TIMER_DIVIDER_REG, 0b11);

    /* Set the intial count to max */
    lapic_write_reg(LAPIC_TIMER_INITIAL_CNT_REG, 0xffffffff);

    /* Set the timer speed in microseconds */
    timer_speed_us = 1000;

    /* Call a delay function based on the available timer */
    pmt_delay(timer_speed_us);

    /* Mask the timer (prevents interrupts) */
    lapic_write_reg(LAPIC_LVT_TIMER_REG, LAPIC_TIMER_MASK);

    /* Determine the inital count to be used for a delay set by `timer_speed_us` */
    uint32_t calibration = 0xffffffff - lapic_read_reg(LAPIC_TIMER_CURRENT_CNT_REG);

    /* Set the timer interrupt vector and put the timer into periodic mode */
    lapic_write_reg(LAPIC_LVT_TIMER_REG, LAPIC_TIMER_VECTOR | LAPIC_TIMER_PERIODIC);

    /* Set the inital count to the calibration */
    lapic_write_reg(LAPIC_TIMER_INITIAL_CNT_REG, calibration);

}


void apic_init(void){
    asm("cli");

    lapic_address = madt->lic_address + hhdmoffset;

    lapic_ao_t *lapic_ao = (lapic_ao_t*) find_ics(0x5); // Local APIC Address Override

    /* If there is a lapic address override present then use that instead */
    if(lapic_ao){
        /* Check that the field isnt 0 */
        if(lapic_ao->lapic_address != 0){
            lapic_address = lapic_ao->lapic_address + hhdmoffset;
        }
    }
    
    /* Enable the lapic and set the spurious interrupt vector to 0xFF */
    lapic_write_reg(LAPIC_SPURIOUS_REG, 0x1FF);

    /* Initialize the IOAPIC */
    ioapic_init();

    /* Start the timers for calibration of the APIC timer */
    timer_init();    

    /* Start the APIC timer */
    lapic_timer_init();
    
    asm("sti");
}

void apic_timer_handler(){
    lapic_write_reg(LAPIC_EOI_REG, 0);
    lapic_timer_ticks++;
}