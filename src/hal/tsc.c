#include <cpuid.h>
#include <stdio.h>
#include <stdint.h>
#include "error.h"
#include "../drivers/pmt.h"

uint32_t core_crystal_clock = 0;

void enable_tsc(){
    asm(".intel_syntax noprefix\n\
        mov rax, cr4\n\
        or rax, 0b10\n\
        mov cr4, rax\n\
        .att_syntax prefix");
}

void disable_tsc(){
    asm(".intel_syntax noprefix\n\
        mov rax, cr4\n\
        and rax, 0xFFFFFFFFFFFFFFFD\n\
        mov cr4, rax\n\
        .att_syntax prefix");
}

uint64_t read_tsc(){
    uint32_t eax, edx;
    asm("rdtsc" :"=a"(eax),"=d"(edx));
    return ((uint64_t)edx << 32 | eax);
}

kstatus tsc_init(){

    uint32_t edx, unused;

    /* Check if there is an invariant TSC */
    __get_cpuid(0x80000007, &unused, &unused, &unused, &edx);

    if((edx & (1 << 8)) == 0){
        return KERNEL_STATUS_ERROR;
    }

    kprintf("tsc: Invariant TSC found\n");

    /* Get the core crystal clock so we can determine TSC speed */
    __get_cpuid(0x15, &unused, &unused, &core_crystal_clock, &unused);

    if(core_crystal_clock != 0){
        kprintf("cpuid 15h supported!\n");

        /* Make it so that it ticks every millisecond */
        core_crystal_clock *= 1000;

    }else{
        /* Calibrate using the PMT */
        enable_tsc();
        uint32_t read1 = read_tsc();
        pmt_delay(1000);
        uint32_t read2 = read_tsc();
        disable_tsc();
        core_crystal_clock = read2 - read1;
    }

    kprintf("Core crystal clock: {d}\n", core_crystal_clock);

    enable_tsc();

    return KERNEL_STATUS_SUCCESS;

}

uint64_t tsc_get_timestamp(){
    if(core_crystal_clock == 0){
        return 0;
    }
    uint64_t read = read_tsc();
    return read / core_crystal_clock;
}