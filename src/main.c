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
    klog(LOG_SUCCESS, "idt", "Done!");;

    klog(LOG_INFO, "acpi", "Reading ACPI tables");
    acpi_init();
    klog(LOG_SUCCESS, "acpi", "Done!");  

    klog(LOG_INFO, "apic", "Initalizing APIC");
    apic_init();
    klog(LOG_SUCCESS, "apic", "Done!");

    tsc_init();  

    klog(LOG_INFO, "pmm", "Setting up the PMM");
    pmm_init();
    klog(LOG_SUCCESS, "pmm", "Done!");

    klog(LOG_INFO, "vmm", "Setting up the page tables");
    vmm_init();
    klog(LOG_SUCCESS, "vmm", "Done!");

    kernel_heap_init();

    klog(LOG_INFO, "smp", "Starting APs");
    smp_init();
    klog(LOG_SUCCESS, "smp", "Done!");

    klog(LOG_INFO, "pci", "Getting le pci");
    pci_init();
    klog(LOG_SUCCESS, "pci", "Done!");

    scheduler_init();

/*     klog(LOG_INFO, "ahci", "Initializing AHCI controller");
    ahci_init();
    klog(LOG_SUCCESS, "ahci", "Done!"); */

    death:
    for(;;);
}

bool kernel_killed = false;
void kkill(void){
    kernel_killed = true;
    asm volatile("cli; hlt");
    for(;;);
}
