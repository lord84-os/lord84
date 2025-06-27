#include <stdint.h>
#include <stddef.h>
#include <lord84.h>
#include <string.h>
#include "../limine/limine.h"
#include "include/stdio.h"
#include "flanterm/flanterm.h"
#include "flanterm/backends/fb.h"
#include "hal/gdt.h"
#include "hal/idt.h"
#include "hal/apic.h"
#include "hal/timer.h"
#include "hal/smp.h"
#include "hal/tsc.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/kmalloc.h"
#include "sys/acpi.h"
#include "sys/pci.h"
#include "drivers/serial.h"
#include "drivers/pmt.h"
#include "drivers/ahci.h"
#include "scheduler/sched.h"
#include <io.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
};

struct flanterm_context *ft_ctx;

uint64_t hhdmoffset = 0;

stack_frame *l84_kpanic_stack_frame;

void _start(void){

    if(hhdm_request.response == NULL){
        goto death;
    }


    hhdmoffset = hhdm_request.response->offset;

    /* initalize framebuffer */
    struct limine_framebuffer_response *fb_response = framebuffer_request.response;

    if(fb_response == NULL){
        goto death;
    }

    struct limine_framebuffer *fb = fb_response->framebuffers[0];

    if(fb == NULL){
        goto death;
    }

    ft_ctx = flanterm_fb_init(
            NULL,
            NULL,
            fb->address, fb->width, fb->height, fb->pitch,
            fb->red_mask_size, fb->red_mask_shift,
            fb->green_mask_size, fb->green_mask_shift,
            fb->blue_mask_size, fb->blue_mask_shift,
            NULL,
            NULL, NULL,
            NULL, NULL,
            NULL, NULL,
            NULL, 0, 0, 1,
            0, 0,
            0
    );

    kprintf("Welcome to lord84{n}");

    extern link_symbol_ptr text_start_addr, text_end_addr;

    klog(LOG_INFO, "serial", "Initalizing serial controller");
    serial_init();
    klog(LOG_SUCCESS, "serial", "Done!");

    klog(LOG_INFO, "gdt", "Setting up the GDT");
    set_gdt();
    klog(LOG_SUCCESS, "gdt", "Done!");

    klog(LOG_INFO, "idt", "Setting up the IDT");
    set_idt();
    klog(LOG_SUCCESS, "idt", "Done!");

    klog(LOG_INFO, "acpi", "Reading ACPI tables");
    acpi_init();
    klog(LOG_SUCCESS, "acpi", "Done!");  

    klog(LOG_INFO, "pmm", "Setting up the PMM");
    pmm_init();
    klog(LOG_SUCCESS, "pmm", "Done!");

    klog(LOG_INFO, "vmm", "Setting up the page tables");
    vmm_init();
    klog(LOG_SUCCESS, "vmm", "Done!");

    kernel_heap_init();

    /* Allocate CPU structure */
    cpu_state *cpu_struct = (cpu_state*)kmalloc(sizeof(cpu_state));

    wrmsr(KERNELGSBASE, (uint64_t)cpu_struct);
    wrmsr(GSBASE, (uint64_t)cpu_struct);

    //sched_init();

    klog(LOG_INFO, "apic", "Initalizing APIC");
    apic_init();
    klog(LOG_SUCCESS, "apic", "Done!");

    tsc_init();  

    l84_kpanic_stack_frame = (stack_frame*)kernel_allocate_memory(sizeof(stack_frame), PTE_BIT_NX | PTE_BIT_RW);

    klog(LOG_INFO, "smp", "Starting APs");
    smp_init();
    klog(LOG_SUCCESS, "smp", "Done!");

    klog(LOG_INFO, "pci", "Getting the pci");
    pci_init();
    klog(LOG_SUCCESS, "pci", "Done!");


    logprintf("Hello from logger!\n");

    //sched_init();

/*     klog(LOG_INFO, "ahci", "Initializing AHCI controller");
    ahci_init();
    klog(LOG_SUCCESS, "ahci", "Done!"); */

    death:
    for(;;);
}

void kkill(void){
    apic_send_ipi((struct ipi){255, 0, 0, 0b1, 0b11}); // Send all other processors to halt state
    asm volatile("cli; hlt");
    for(;;);
}

void get_stack_frame(stack_frame *r);

void kpanic(const char *reason){

    get_stack_frame(l84_kpanic_stack_frame);

    kprintf("{k}Panic! on CPU {d} {k}\n", ANSI_COLOR_RED, get_cpu_struct()->lapic_id, ANSI_COLOR_RESET);
    kprintf("reason: '{s}'\n", reason);
    kprintf("at ip: 0x{x}\n\n", l84_kpanic_stack_frame->rip);
    kprintf("rax 0x{x} | rbx 0x{x} | rcx 0x{x} | rdx 0x{xn}", l84_kpanic_stack_frame->rax, l84_kpanic_stack_frame->rbx, l84_kpanic_stack_frame->rcx, l84_kpanic_stack_frame->rdx);
    kprintf("rdi 0x{x} | rsi 0x{x} | rbp 0x{xn}", l84_kpanic_stack_frame->rdi, l84_kpanic_stack_frame->rsi, l84_kpanic_stack_frame->rbp);
    kprintf("r8 0x{x} | r9 0x{x} | r10 0x{x} | r11 0x{x} | r12 0x{x} | r13 0x{x} | r14 0x{x} | r15 0x{xn}", l84_kpanic_stack_frame->r8, l84_kpanic_stack_frame->r9, l84_kpanic_stack_frame->r10, l84_kpanic_stack_frame->r11, l84_kpanic_stack_frame->r12, l84_kpanic_stack_frame->r13, l84_kpanic_stack_frame->r14, l84_kpanic_stack_frame->r15);
    kprintf("cs 0x{x} | ss 0x{x} | rsp 0x{x}\n", l84_kpanic_stack_frame->cs, l84_kpanic_stack_frame->ss, l84_kpanic_stack_frame->rsp);
    kprintf("cr2: 0x{x}\n", l84_kpanic_stack_frame->cr2);

    kkill();
    for(;;);
}
