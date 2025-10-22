#include "../sys/acpi.h"
#include "../hal/ioapic.h"
#include "../hal/apic.h"
#include "../drivers/pmt.h"
#include "timer.h"
#include <stdio.h>
#include <SFB25.h>

/* Determines which timer will be used for calibration */
int calibration_timer = -1;

void timer_init(void){
    if(pmt_init() == -1){
        klog(LOG_INFO, __func__, "PMT Timer not found, falling back");
        /* Fall back to PIT */
    }else{
        calibration_timer = PMT;
    }
}

void sleep(int ms){
    /* Eventually fix this */
    apic_sleep(ms);
}