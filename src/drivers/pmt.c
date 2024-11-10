#include "../sys/acpi.h"
#include <stdio.h>
#include <lord84.h>
#include <io.h>


#define PMT_TIMER_RATE 3579545

#define X_PMT_TMR_BLOCK_OFFSET 208 

fadt_t *fadt;
extern uint64_t hhdmoffset;

/* Use extended address fields instead */
bool use_ext = false;

uint64_t pmt_read_reg(gas_t X_PMTimerBlock){
    
    /* TO FIX - address space id invalid in Bochs */
    
    /* Check address space ID field to understand how to access the register */
    if(X_PMTimerBlock.address_space_id == 0x00){
        /* Access through memory */
        return *((uint64_t*)X_PMTimerBlock.address);
    }else if (X_PMTimerBlock.address_space_id == 0x01){
        /* Access through I/O port */
        return inl(X_PMTimerBlock.address);
    }else{
        serial_kprintf("address id: 0x{xn}", X_PMTimerBlock.address_space_id);
        klog(LOG_ERROR, __func__, "X_PMTimerBlock address space id isn't supported!");
        return 0;
    }
}

int pmt_init(){
    fadt = (fadt_t*)((uint64_t)find_acpi_table("FACP") + hhdmoffset);

    if(!fadt){
        klog(LOG_ERROR, __func__, "Didn't find FADT table");
        kkill();
    }

    /* Check if timer exists */
    if(fadt->PMTimerLength == 0){
        return -1;
    }

    /* If ACPI revision is over or equal 2 and if X_PMTimerBlock isnt 0, then use X_PMTimerBlock */
    if(fadt->header.revision >= 2 && fadt->X_PMTimerBlock.address != 0 && (fadt->header.length >= X_PMT_TMR_BLOCK_OFFSET)){
        serial_kprintf("pmt: Using the X_PMTimerBlock\n");

        use_ext = true;
    }

    return 0;
}

void pmt_delay(uint64_t us){
    uint64_t count;

    if(!use_ext){
        count = inl(fadt->PMTimerBlock);
    }else{
        count = pmt_read_reg(fadt->X_PMTimerBlock);
    }

    uint64_t target = (us * PMT_TIMER_RATE) / 1000000;
    uint64_t current = 0;

    while(current < target){
        if(!use_ext){
            current = ((inl(fadt->PMTimerBlock) - count) & 0xffffff);
        }else{
            current = (pmt_read_reg(fadt->X_PMTimerBlock) - count) & 0xffffff;
        }
    }
}