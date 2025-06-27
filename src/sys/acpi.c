#include <limine.h>
#include <stddef.h>
#include <stdio.h>
#include <lord84.h>
#include <string.h>
#include <stdalign.h>
#include "acpi.h"

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

extern uint64_t hhdmoffset;

xsdt_t *xsdt;
rsdt_t *rsdt;
madt_t *madt;

/* Returns pointer to the table with specified signature, if it doesnt find it then it returns NULL */
uint64_t *find_acpi_table(char *signature){
    
    uint64_t entries = 0; // stores the total number of entries in the table
    
    if(xsdt){
        /* The total number of entries is the length of the entire table minus the standard header from the xsdt and divided by 8, as the array is of 8 byte wide headers*/
        entries = (xsdt->header.length - sizeof(desc_header_t)) / 8;
    }else{
        entries = (rsdt->header.length - sizeof(desc_header_t)) / 4;
    }

    desc_header_t *header;

    for(uint64_t i = 0; i < entries; i++){
        if(xsdt){
            header = (desc_header_t*)(xsdt->entries_base[i]);
        }else{
            header = (desc_header_t*)(rsdt->entries_base[i]);
        }

        /* Get the virtual address of the header so we can access its signature */
        desc_header_t *virt = (desc_header_t*)((uint64_t)header + hhdmoffset);

        if(memcmp(virt->signature, signature, 4) == 0){
            return (uint64_t*)header;
        }
    }

    return NULL;
    
}

void acpi_init(void){

    if(rsdp_request.response == NULL){
        klog(LOG_ERROR, "acpi", "RSDP request is NULL");
        kkill();
    }

    rsdp_t *rsdp = (rsdp_t*)(rsdp_request.response->address);

    logprintf("RSDP address: 0x{x}\n", (uint64_t)(rsdp));

    /* If the systems ACPI revision is higher/equal than 2, then use XSDT */
    if(rsdp->revision >= 2){
        rsdt = NULL;
        xsdt = (xsdt_t*)(rsdp->xsdt_address + hhdmoffset);
        klog(LOG_INFO, "acpi", "Using XSDT header");
        kprintf("XSDT address: 0x{xn}", (uint64_t)xsdt);
        kprintf("OEMID: {ccccccn}", xsdt->header.oemid[0], xsdt->header.oemid[1], xsdt->header.oemid[2], xsdt->header.oemid[3], xsdt->header.oemid[4], xsdt->header.oemid[5]);
    }else{
        xsdt = NULL;
        rsdt = (rsdt_t*)(rsdp->rsdt_address + hhdmoffset);
        klog(LOG_INFO, "acpi", "Using RSDT header");
        kprintf("RSDT address: 0x{xn}", (uint64_t)rsdt);
        kprintf("OEMID: {ccccccn}", rsdt->header.oemid[0], rsdt->header.oemid[1], rsdt->header.oemid[2], rsdt->header.oemid[3], rsdt->header.oemid[4], rsdt->header.oemid[5]);
    }

    madt = (madt_t*)find_acpi_table("APIC");
    madt = (madt_t*)((uint64_t)madt + hhdmoffset);


    if(!madt){
        klog(LOG_ERROR, __func__, "MADT table not found");
        kkill();
    }
}

uint64_t *find_ics(uint64_t type){
    uint64_t length = (madt->header.length - sizeof(desc_header_t) - 8);
    uint64_t *base_addr = (uint64_t*)madt->ics;

    uint64_t i = 0;
    while (i < length) {
        ics_t *header = (ics_t*)((uint64_t)base_addr + i);
        if(header->type == type){
            return (uint64_t*)header;
        }
        i += header->length;
    }

    return NULL;
}

uint32_t find_iso(uint8_t legacy){
    uint64_t length = (madt->header.length - sizeof(desc_header_t) - 8);
    uint64_t *base_addr = (uint64_t*)madt->ics;

    uint64_t i = 0;
    while (i < length) {
        ics_t *header = (ics_t*)((uint64_t)base_addr + i);
        if(header->type == 0x2){
            iso_t *iso = (iso_t*)header;
            if(legacy == iso->source){
                return iso->gsi;
            }
        }
        i += header->length;
    }

    /* GSI is equal to legacy pin */
    return legacy;
}