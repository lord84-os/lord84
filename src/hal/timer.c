#include "../sys/acpi.h"
#include "../hal/ioapic.h"
#include <stdio.h>
#include <lord84.h>

#define HPET_GENERAL_CPB_REG    0x0
#define HPET_GENERAL_CFG_REG    0x10
#define HPET_GENERAL_INT_REG    0x20

#define HPET_MAIN_COUNTER       0xf0

#define HPET_TIMER_CFG_REG(n)   (0x20 * n) + 0x100
#define HPET_TIMER_CMP_REG(n)   (0x20 * n) + 0x108

#define HPET_INT_ENABLE         (1 << 2)
#define HPET_INT_DISABLE        0xFFFFFFFFFFFFFFFB

#define HPET_ENABLE             (1 << 0)
#define HPET_DISABLE            0xFFFFFFFFFFFFFFFE

bool called = false;

uint64_t hpet_address = 0;
uint64_t hpet_frequency = 0;
uint64_t hpet_num_timers = 0;

uint8_t int_pin[32] = {255};

extern uint64_t hhdmoffset;

void hpet_write_reg(uint64_t reg, uint64_t data){
    *(uint64_t*)(hpet_address + reg) = data;
}

uint64_t hpet_read_reg(uint64_t reg){
    return *(uint64_t*)(hpet_address + reg);
}

void hpet_enable_timer_interrupt(uint8_t timer){
    hpet_write_reg(HPET_TIMER_CFG_REG(timer), hpet_read_reg(HPET_TIMER_CFG_REG(timer)) | HPET_INT_ENABLE);
}

void hpet_disable_timer_interrupt(uint8_t timer){
    hpet_write_reg(HPET_TIMER_CFG_REG(timer), hpet_read_reg(HPET_TIMER_CFG_REG(timer)) & HPET_INT_DISABLE);
}

void hpet_sleep(uint64_t ms){
    /* First disable the timers */
    hpet_write_reg(HPET_GENERAL_CFG_REG, hpet_read_reg(HPET_GENERAL_CFG_REG) & HPET_DISABLE);

    //hpet_disable_timer_interrupt(0);

    /* Write the value into the comparators */
    for(uint64_t i = 0; i < hpet_num_timers; i++){
        hpet_write_reg(HPET_TIMER_CMP_REG(i), hpet_frequency * ms);
    }

    /* Set the counter to zero */
    hpet_write_reg(HPET_MAIN_COUNTER, 0);

    //hpet_enable_timer_interrupt(0);

    /* Enable the timers */
    hpet_write_reg(HPET_GENERAL_CFG_REG, hpet_read_reg(HPET_GENERAL_CFG_REG) | HPET_ENABLE);

    while(!called){
        asm volatile("nop");
    }

    called = false;
    
    kprintf("smort");
}

void hpet_handler(){
    called = true;
    kprintf("frequency: 0x{xn}comparator: 0x{xn}counter: 0x{xn}", hpet_frequency, hpet_read_reg(HPET_TIMER_CMP_REG(0)), hpet_read_reg(HPET_MAIN_COUNTER));
    return;
}

int hpet_init(void){
    hpet_t *hpet = (hpet_t*)find_acpi_table("HPET");

    if(!hpet){
        return -1;
    }

    hpet_address = hpet->base_address.address + hhdmoffset;

    kprintf("hpet: HPET Address: 0x{x}\n", hpet_address);
    
    kprintf("CFG: 0b{bn}", hpet_read_reg(HPET_GENERAL_CFG_REG));

    /* Set LEG_RT_CNF (legacy replacement) bit to 0, so we can use standard mapping */
    hpet_write_reg(HPET_GENERAL_CFG_REG, hpet_read_reg(HPET_GENERAL_CFG_REG) & 0xFFFFFFFFFFFFFFFC);

    /* Get the number of timers available. */
    hpet_num_timers = (hpet_read_reg(HPET_GENERAL_CPB_REG) >> 8) & 0xF;

    /* The value in the timers variable represents the last timer, thats why we add 1 to it to get the total number of timers */
    hpet_num_timers += 1;

    if(hpet_num_timers > 32 || hpet_num_timers < 3){
        klog(LOG_ERROR, __func__, "Invalid HPET timer count");
        kkill();
    }

    /* Disable timer interrupts */
    for(uint64_t i = 0; i < hpet_num_timers; i++){
        hpet_disable_timer_interrupt(i);
    }

    kprintf("hpet: Number of timers: {d}\n", hpet_num_timers);

    /* Upper 32 bits are a bitfield which determine which interrupts can be used */
    uint32_t routing_capabilities; 

    for(uint64_t i = 0; i < hpet_num_timers; i++){
        /* Get the bitfield so we can parse the interrupt pins */
        routing_capabilities = hpet_read_reg(HPET_TIMER_CFG_REG(i)) >> 32;
        kprintf("routing capable: 0b{bn}", routing_capabilities);

        /* Check each bit in the bitfield if its set */
        for(uint64_t j = 0; j < 32; j++){
            if((routing_capabilities & (1 << j)) > 0){
                int_pin[i] = j;
                break;
            }
        }
    }

    

    for(uint32_t i = 0; i < hpet_num_timers; i++){
        kprintf("int_pin[{d}]: {dn}", i, int_pin[i]);
    }

    /* Calculate frequency (10^15 / period) */
    uint64_t period = hpet_read_reg(HPET_GENERAL_CPB_REG) >> 32;
    hpet_frequency = 1000000000000000 / period;

    kprintf("hpet: Frequency is: {d}Hz\n", hpet_frequency);

    /* Set value of comparator */
    for(uint64_t i = 0; i < hpet_num_timers; i++){
        hpet_write_reg(HPET_TIMER_CMP_REG(i), 1000000000000000 * period);
    }

    /* Set the interrupt vector for all timers */
    for(uint64_t i = 0; i < hpet_num_timers; i++){
        hpet_write_reg(HPET_TIMER_CFG_REG(i), hpet_read_reg(HPET_TIMER_CFG_REG(i)) | (uint64_t)((int_pin[i] & 0xF) << 9)); // problem
        serial_kprintf("timer {d} cfg = 0b{bn}", i, hpet_read_reg(HPET_TIMER_CFG_REG(i)));
    }

    hpet_enable_timer_interrupt(0);

    write_redir_entry(int_pin[0], 44);

    /* Enable timers */
    hpet_write_reg(HPET_GENERAL_CFG_REG, hpet_read_reg(HPET_GENERAL_CFG_REG) | HPET_ENABLE);

    return 0;
}

int pmt_init(){

}

void timer_init(void){
    if(hpet_init() == -1){
        klog(LOG_WARN, __func__, "HPET not found, falling back");
    }
/*     uint64_t time = 0;
    uint64_t *timer_count = (uint64_t*)(hpet_address + HPET_MAIN_COUNTER);
    *timer_count = *timer_count + time; */
/*     for(;;){
        //serial_kprintf("COUNT: 0x{xn}", *(uint64_t*)(hpet_address + HPET_MAIN_COUNTER));
    }   */


}