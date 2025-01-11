#include <limine.h>
#include <stdio.h>
#include <lord84.h>
#include "gdt.h"
#include "apic.h"
#include "idt.h"
#include "../mm/vmm.h"
#include <lock.h>
#include <io.h>

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

extern void s_load_idt();
extern void s_load_gdt();

void ap_init(struct limine_smp_info *smp_info){

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

    /* Initialize APIC & APIC timer */
    ap_apic_init();

    kprintf("smp: initialized CPU {d}\n", smp_info->lapic_id);

    for(;;){
        asm("cli");
        asm("hlt");
    }
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

    /* If one of the APs has halted, then halt the BSP */
    extern bool kernel_killed;
    if(kernel_killed == true){
        kkill();
    }

}