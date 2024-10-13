#include <limine.h>
#include <stddef.h>
#include <stdio.h>
#include <lord84.h>
#include <string.h>
#include "acpi.h"

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

extern uint64_t hhdmoffset;

xsdt_t *xsdt;
rsdt_t *rsdt;

/* Returns pointer to the table with specified signature, if it doesnt find it then it returns NULL */
uint64_t *find_table(char *signature, bool use_xsdt){
    
    uint64_t entries = 0; // stores the total number of entries in the table
    
    if(use_xsdt == true){
        /* The total number of entries is the length of the entire table minus the standard header from the xsdt and divided by 8, as the array is of 8 byte wide headers*/
        entries = (xsdt->header.length - sizeof(desc_header_t)) / 8;
    }else{
        entries = (rsdt->header.length - sizeof(desc_header_t)) / 4;
    }

    desc_header_t *header;

    for(uint64_t i = 0; i < entries; i++){

        if(use_xsdt == true){
            header = (desc_header_t*)(xsdt->entries_base[i]);
        }else{
            header = (desc_header_t*)(rsdt->entries_base[i]);
        }

        if(!memcmp(header->signature, signature, 4)){
            return (uint64_t*)(header);
        }
    }

    return NULL;
    
}

void acpi_init(void){

    bool use_xsdt = false;

    if(rsdp_request.response == NULL){
        klog(LOG_ERROR, "acpi", "RSDP request is NULL");
        kkill();
    }

    rsdp_t *rsdp = (rsdp_t*)(rsdp_request.response->address);

    kprintf("RSDP address: 0x{xn}", (uint64_t)(rsdp));

    /* If the systems ACPI revision is higher/equal than 2, then use XSDT */
    if(rsdp->revision >= 2){
        use_xsdt = true;
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
}