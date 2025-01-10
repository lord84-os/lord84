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

    kprintf("Hello from processor {d}\n", smp_info->lapic_id);
    kprintf("1 PAT: 0x{xn}", rdmsr(0x277));
    serial_kprintf("1 PAT: 0x{xn}", rdmsr(0x277));

    /* Set the CR3 context */
    extern uint64_t *kernel_page_map;

    vmm_set_ctx(kernel_page_map);

    for(;;){
        asm("cli");
        asm("hlt");
    }

    asm volatile(
        "movq %%cr3, %%rax\n\
	   movq %%rax, %%cr3\n"
        : : : "rax"
   );








    /* Load the APIC */
    apic_init();

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
        if(smp_response->cpus[i]->lapic_id == 0){
            kprintf("0 PAT: 0x{xn}", rdmsr(0x277));
            serial_kprintf("0 PAT: 0x{xn}", rdmsr(0x277));
        }
        smp_response->cpus[i]->goto_address = &ap_init;
    }

}