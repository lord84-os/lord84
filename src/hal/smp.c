#include <limine.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <lord84.h>
#include "gdt.h"
#include "smp.h"
#include "apic.h"
#include "idt.h"
#include "../mm/vmm.h"
#include <lock.h>
#include <io.h>
#include <string.h>

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

extern void s_load_idt();
extern void s_load_gdt();
extern uint64_t hhdmoffset;

/* Returns the CPU structure for this particular CPU */
cpu_state *get_cpu_struct(){
    return (cpu_state*)rdmsr(GSBASE);
}

uint64_t get_cpu_count(){
    return smp_request.response->cpu_count;
}

bool get_cpu_struct_initialized(){
    if(rdmsr(GSBASE) < hhdmoffset){
        return false;
    }

    return true;
}

atomic_flag ap_init_lock = ATOMIC_FLAG_INIT;

void ap_init(struct limine_smp_info *smp_info){

    acquire_lock(&ap_init_lock);

    /* Load the GDT */
    s_load_gdt();

    /* Load the IDT */
    s_load_idt();

    /* Set the CR3 context */
    extern uint64_t *kernel_page_map;

    vmm_set_ctx(kernel_page_map);

    asm volatile(
        "movq %%cr3, %%rax\n\
    movq %%rax, %%cr3\n"
        : : : "rax"
    );

    cpu_state *cpu_struct = (cpu_state*)kmalloc(sizeof(cpu_state));
    memset(cpu_struct, 0, sizeof(cpu_state));
    cpu_struct->lapic_id = smp_info->lapic_id;

    wrmsr(KERNELGSBASE, (uint64_t)cpu_struct);
    wrmsr(GSBASE, (uint64_t)cpu_struct);

    /* Initialize APIC & APIC timer */
    ap_apic_init();

    free_lock(&ap_init_lock);

    for(;;);

    scheduler_init();
}

void smp_init(){
    
    if(!smp_request.response){
        klog(LOG_ERROR, __func__, "Failed to get SMP request");
        kkill();
    }

    struct limine_smp_response *smp_response = smp_request.response;

    kprintf("smp: {d} CPUs\n", smp_response->cpu_count);

    for(uint64_t i = 0; i < smp_response->cpu_count; i++){
        /* Pointer to smp_info is passed in RDI by Limine, so no need to pass any arguments here */
        smp_response->cpus[i]->goto_address = &ap_init;
    }

    /* -- Setup CPU structure for BSP -- */

    /* Allocate CPU structure */
    cpu_state *cpu_struct = (cpu_state*)kmalloc(sizeof(cpu_state));
    cpu_struct->lapic_id = smp_response->cpus[0]->lapic_id;

    wrmsr(KERNELGSBASE, (uint64_t)cpu_struct);
    wrmsr(GSBASE, (uint64_t)cpu_struct);

    /* If one of the APs has halted, then halt the BSP */
    extern bool kernel_killed;
    if(kernel_killed == true){
        kkill();
    }

}