#include "../sys/acpi.h"
#include <stdio.h>

#define HPET_GENERAL_CPB_REG    0x0
#define HPET_GENERAL_CFG_REG    0x10

#define HPET_TIMER_CFG_REG(n)   (0x20 * n) + 0x100

uint64_t hpet_address;
extern uint64_t hhdmoffset;

int hpet_init(void){
    hpet_t *hpet = (hpet_t*)find_acpi_table("HPET");

    if(!hpet){
        return -1;
    }

    hpet_address = hpet->base_address.address + hhdmoffset;

    uint64_t cfg_r = *(uint64_t*)(hpet_address+HPET_GENERAL_CFG_REG);

    kprintf("HPET Address: 0x{x}\n", hpet_address);
    
    /* Set LEG_RT_CNF (legacy replacement) bit to 0, so we can use standard mapping */
    *(uint64_t*)(hpet_address+HPET_GENERAL_CFG_REG) = cfg_r & 0xFFFFFFFFFFFFFFFD;

    uint64_t timers = ((*(uint64_t*)(hpet_address+HPET_GENERAL_CPB_REG)) & 0b0000000000000000000000000000000000000000000000000000011110000000) >> 7;

    kprintf("Number of timers: {d}\n", timers);
    /* Stores the interrupt which the timers will use for each of the three timers */
    uint8_t int_pin[4] = {0};
    /* Upper 32 bits are a bitfield which determine which interrupts can be used */
    uint32_t routing_capabilities; 

    for(uint64_t i = 0; i < timers; i++){
        /* Get the bitfield so we can parse the interrupt pins */
        routing_capabilities = *(uint64_t*)((hpet_address+HPET_TIMER_CFG_REG(i)) >> 32);
        kprintf("routing: 0b{b}\n", routing_capabilities);

        /* Check each bit in the bitfield if its set */
        for(uint64_t j = 0; j < 32; j++){
            if((routing_capabilities & (1 << j)) > 0){
                
                /* Make sure that none of the interrupts from the other timers are the same */
                if(j == int_pin[0] || j == int_pin[1] || j == int_pin[2] || j == int_pin[3]){
                    continue;
                }

                int_pin[i] = j;
                break;
            }
        }
    }


    kprintf("int_pin[0]: 0x{x}\nint_pin[1]: 0x{x}\nint_pin[2]: 0x{x}\nint_pin[3]: 0x{x}\n", int_pin[0], int_pin[1], int_pin[2], int_pin[3]);

    return 0;
}

void timer_init(void){
    if(hpet_init() == -1){
        klog(LOG_WARN, __func__, "HPET not found, falling back");
    }

    

}