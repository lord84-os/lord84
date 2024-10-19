#include <stdint.h>
#include <stdio.h>
#include <lord84.h>
#include <string.h>
#include <limine.h>
#include "pmm.h"
#include "vmm.h"

#define PAGE_SIZE   0x1000

static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
};

extern uint64_t hhdmoffset;

typedef uint64_t pml4e, pdpe, pde, pte;

/* Initial PML4 address & CR3 */
uint64_t *kpage_map;

void vmm_init(){
    
    /* Find an appropriate place for the page tables */
    kpage_map = pmm_alloc();

    if(!kpage_map){
        klog(LOG_ERROR, __func__, "Failed to allocate page for page map");
        kkill();
    }

    kprintf("vmm: Kernel page map is at: 0x{x}\n", (uint64_t)kpage_map);

    struct limine_kernel_address_response *kernel_address = kernel_address_request.response;

    if(!kernel_address){
        klog(LOG_ERROR, __func__, "Kernel address request is null");
        kkill();
    }

    extern link_symbol_ptr text_start_addr, text_end_addr,
    rodata_start_addr, rodata_end_addr,
    data_start_addr, data_end_addr;

    uint64_t text_start = ALIGN_DOWN((uint64_t)text_start_addr, PAGE_SIZE),
    rodata_start = ALIGN_DOWN((uint64_t)rodata_start_addr, PAGE_SIZE),
    data_start = ALIGN_DOWN((uint64_t)data_start_addr, PAGE_SIZE),
    text_end = ALIGN_UP((uint64_t)text_end_addr, PAGE_SIZE),
    rodata_end = ALIGN_UP((uint64_t)rodata_end_addr, PAGE_SIZE),
    data_end = ALIGN_UP((uint64_t)data_end_addr, PAGE_SIZE); 

    kprintf("test1\n");

    /* Map all linker segments */
    for(uint64_t text = text_start; text < text_end; text += PAGE_SIZE){
        uint64_t phys = text - kernel_address->virtual_base;
        kprintf("text: 0x{x}\nphys:: 0x{x}\n", text, phys);
        vmm_map_page(kpage_map, (uint64_t*)text, (uint64_t*)phys, PTE_BIT_PRESENT);
    }

    for(uint64_t data = data_start; data < data_end; data += PAGE_SIZE){
        uint64_t phys = data - kernel_address->virtual_base;
        vmm_map_page(kpage_map, (uint64_t*)data, (uint64_t*)phys, PTE_BIT_PRESENT | PTE_BIT_NX | PTE_BIT_RW);
    }

    for(uint64_t rodata = rodata_start; rodata < rodata_end; rodata += PAGE_SIZE){
        uint64_t phys = rodata - kernel_address->virtual_base;
        vmm_map_page(kpage_map, (uint64_t*)rodata, (uint64_t*)phys, PTE_BIT_PRESENT | PTE_BIT_NX);
    }


    extern struct limine_memmap_response *memmap_response;

    struct limine_memmap_entry **entries = memmap_response->entries;

    for(uint64_t i = 0; i < memmap_response->entry_count; i++){
        uint64_t base = ALIGN_DOWN(entries[i]->base, PAGE_SIZE);
        uint64_t length = ALIGN_UP(entries[i]->length, PAGE_SIZE);

        kprintf("base: 0x{x}, length: 0x{xn}",base, length);

        switch (entries[i]->type) {
            case LIMINE_MEMMAP_USABLE:
                for(uint64_t j = 0; j < length; j += PAGE_SIZE){
                    vmm_map_page(kpage_map, (uint64_t*)(base+j+hhdmoffset), (uint64_t*)(base+j), PTE_BIT_PRESENT | PTE_BIT_RW);
                }
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                for(uint64_t j = 0; j < length; j += PAGE_SIZE){
                    vmm_map_page(kpage_map, (uint64_t*)(base+j+hhdmoffset), (uint64_t*)(base+j), PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);
                }
                break;              
        }
    }

    kprintf("Setting context\n");
    vmm_set_ctx(kpage_map);

    tlb_flush();
}

void tlb_flush(void){
    asm volatile(
        "movq %%cr3, %%rax\n\
	    movq %%rax, %%cr3\n"
        : : : "rax"
   );
}

void vmm_map_page(uint64_t *page_map, uint64_t *virt_address, uint64_t* phys_address, uint64_t flags){

    uint64_t address = (uint64_t)virt_address;

    uint8_t pml4_index = (address >> 39) & 0xFF;
    uint8_t pdp_index = (address >> 30) & 0xFF;
    uint8_t pd_index = (address >> 21) & 0xFF;
    uint8_t pt_index = (address >> 12) & 0xFF;
    uint8_t pp_index = (address) & 0x7FF;

    /* Page directory pointer */
    
    uint64_t *pdp, *pd, *pt, *pp;

    //kprintf("virt_address: 0x{x}\nphys_address: 0x{x}\n",(uint64_t)virt_address, (uint64_t)phys_address);

    /* First check if this table is already mapped */
    if(((*(uint64_t*)((uint64_t)page_map + hhdmoffset + pml4_index * 8)) & PTE_BIT_PRESENT) != 0){
        /* If this table is already mapped then just point to it*/
        pdp = (uint64_t*)((uint64_t)page_map + pml4_index * 8);
    }else{
        /* Allocate memory for the next table we want to add */
        pdp = pmm_alloc();

        if(!pdp){
            klog(LOG_ERROR, __func__, "Failed to allocate memory for pdp");
            return;
        }

        /* We set it all to zero (remember, each page table no matter the type is 4096 bytes wide)*/
        memset((uint64_t*)((uint64_t)pdp + hhdmoffset), 0, PAGE_SIZE);
    }

    /* Set the value at index `pml4_index` in the `pml4` table (page_map points to the pml4 table) to the address of the next table and then repeat for all the tables */
    *(uint64_t*)((uint64_t)page_map + hhdmoffset + pml4_index * 8) = (uint64_t)pdp | PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_US;

    /* Page directory */
    if(((*(uint64_t*)((uint64_t)pdp + hhdmoffset + pdp_index * 8)) & PTE_BIT_PRESENT) != 0){
        pd = (uint64_t*)((uint64_t)pdp + pdp_index * 8);
    }else{
        pd = pmm_alloc();

        if(!pd){
            klog(LOG_ERROR, __func__, "Failed to allocate memory for pdp");
            return;
        }

        memset((uint64_t*)((uint64_t)pd + hhdmoffset), 0, PAGE_SIZE);
    }

    *(uint64_t*)((uint64_t)pdp + hhdmoffset + pdp_index * 8) = (uint64_t)pd | PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_US;

    if(((*(uint64_t*)((uint64_t)pd + hhdmoffset + pd_index * 8)) & PTE_BIT_PRESENT) != 0){
        pt = (uint64_t*)((uint64_t)pd + pd_index * 8);
    }else{
        pt = pmm_alloc();

        if(!pt){
            klog(LOG_ERROR, __func__, "Failed to allocate memory for pdp");
            return;
        }

        memset((uint64_t*)((uint64_t)pt + hhdmoffset), 0, PAGE_SIZE);
    }

    *(uint64_t*)((uint64_t)pd + hhdmoffset + pd_index * 8) = (uint64_t)pt | PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_US;

    /* Physical page */
    if(((*(uint64_t*)((uint64_t)pt + hhdmoffset + pt_index * 8)) & PTE_BIT_PRESENT) != 0){
        pp = (uint64_t*)((uint64_t)pt + pt_index * 8);
    }else{
        pp = pmm_alloc();

        if(!pp){
            klog(LOG_ERROR, __func__, "Failed to allocate memory for pdp");
            return;
        }

        memset((uint64_t*)((uint64_t)pp + hhdmoffset), 0, PAGE_SIZE);
    }

    *(uint64_t*)((uint64_t)pt + hhdmoffset + pt_index * 8) = (uint64_t)pp | PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_US;

    *(uint64_t*)((uint64_t)pp + hhdmoffset + pp_index * 8) = (uint64_t)phys_address | flags;

    //kprintf("mapped 0x{x} to 0x{xn}", virt_address, phys_address);

    /* TLB flush */
    tlb_flush();

}

void vmm_set_ctx(uint64_t* page_map){

    uint64_t cr3 = 0;

    cr3 = (uint64_t)page_map & 0xFFFFFFFFFFFFF000;

    kprintf("Writing 0x{xn} to CR3", (uint64_t)cr3);

    __asm__ volatile (
        "movq %0, %%cr3\n"
        : : "r" (cr3) : "memory"
    );

}