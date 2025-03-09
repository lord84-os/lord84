/* Time keeping */

#include <stdint.h>
#include "../hal/tsc.h"
uint64_t kernel_get_timestamp(){
    uint64_t ret = 0;
    uint64_t tsc = tsc_get_timestamp();
    
    if(tsc == 0){
        /* Get APIC timestamp */
        
    }

    return tsc;

}