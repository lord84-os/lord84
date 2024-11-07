#include "../sys/acpi.h"
#include "../hal/ioapic.h"
#include "../drivers/pmt.h"
#include <stdio.h>
#include <lord84.h>

void timer_init(void){
    if(pmt_init() == -1){
        klog(LOG_INFO, __func__, "PMT Timer not found, falling back");
        /* Fall back to PIT */
    }

}