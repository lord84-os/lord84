#include <stdint.h>
#include <stdio.h>
#include <lord84.h>
#include <string.h>
#include <limine.h>
#include "pmm.h"
#include "vmm.h"


#define PAGE_SIZE 4096

struct limine_kernel_address_request kernel_addr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

struct limine_kernel_address_response *kernel_address;

extern uint64_t hhdmoffset;

uint64_t *kernel_page_map = 0;
uint64_t kernel_virt = 0;

void vmm_set_ctx(uint64_t *page_map){

    __asm__ volatile (
        "movq %0, %%cr3\n"
        : : "r" ((uint64_t *)((uint64_t)(page_map) - hhdmoffset)) : "memory"
    );

}


void vmm_init(){
    
    struct limine_kernel_address_response *kernel_address = kernel_addr_request.response;
    if(!kernel_address){
        klog(LOG_ERROR, __func__, "Kernel address not recieved");
    }

    kernel_page_map = (uint64_t*)((uint64_t)pmm_alloc() + hhdmoffset);

    if(!kernel_page_map){
        klog(LOG_ERROR, __func__, "Allocating block for page map failed");
    }

    memset(kernel_page_map, 0, PAGE_SIZE);

    //kernel_virt = (uint64_t)(kernel_address) + hhdmoffset; // virtual address of kernel page map

    // map kernel, stolen
    extern link_symbol_ptr text_start_addr, text_end_addr,
        rodata_start_addr, rodata_end_addr,
        data_start_addr, data_end_addr;

    uint64_t text_start = ALIGN_DOWN((uint64_t)text_start_addr, PAGE_SIZE),
        rodata_start = ALIGN_DOWN((uint64_t)rodata_start_addr, PAGE_SIZE),
        data_start = ALIGN_DOWN((uint64_t)data_start_addr, PAGE_SIZE),
        text_end = ALIGN_UP((uint64_t)text_end_addr, PAGE_SIZE),
        rodata_end = ALIGN_UP((uint64_t)rodata_end_addr, PAGE_SIZE),
        data_end = ALIGN_UP((uint64_t)data_end_addr, PAGE_SIZE); 


    // map usable entries, framebuffer and bootloader reclaimable shit
    extern struct limine_memmap_response *memmap_response;
    for(uint64_t i = 0; i < memmap_response->entry_count; i++){
        if(memmap_response->entries[i]->type == LIMINE_MEMMAP_USABLE){
            for(uint64_t j = 0; j < memmap_response->entries[i]->length; j+=PAGE_SIZE){
                vmm_map_page(kernel_page_map, memmap_response->entries[i]->base+j+hhdmoffset, memmap_response->entries[i]->base+j, PTE_BIT_PRESENT | PTE_BIT_RW);
            }
        }
        if(memmap_response->entries[i]->type == LIMINE_MEMMAP_FRAMEBUFFER){
            for(uint64_t j = 0; j < memmap_response->entries[i]->length; j+=PAGE_SIZE){
                vmm_map_page(kernel_page_map, memmap_response->entries[i]->base+j+hhdmoffset, memmap_response->entries[i]->base+j, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);
            }
        }
        if(memmap_response->entries[i]->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE){
            for(uint64_t j = 0; j < memmap_response->entries[i]->length; j+=PAGE_SIZE){
                vmm_map_page(kernel_page_map, memmap_response->entries[i]->base+j+hhdmoffset, memmap_response->entries[i]->base+j, PTE_BIT_PRESENT | PTE_BIT_RW);
            }
        }
        if(memmap_response->entries[i]->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE){
            for(uint64_t j = 0; j < memmap_response->entries[i]->length; j+=PAGE_SIZE){
                vmm_map_page(kernel_page_map, memmap_response->entries[i]->base+j+hhdmoffset, memmap_response->entries[i]->base+j, PTE_BIT_PRESENT | PTE_BIT_RW);
            }
        }

    }

    for (uintptr_t text_addr = text_start; text_addr < text_end; text_addr += PAGE_SIZE) {
        uintptr_t phys = text_addr - kernel_address->virtual_base + kernel_address->physical_base;
        vmm_map_page(kernel_page_map, text_addr, phys, PTE_BIT_PRESENT);
    }

    for (uintptr_t rodata_addr = rodata_start; rodata_addr < rodata_end; rodata_addr += PAGE_SIZE) {
        uintptr_t phys = rodata_addr - kernel_address->virtual_base + kernel_address->physical_base;
        vmm_map_page(kernel_page_map, rodata_addr, phys, PTE_BIT_PRESENT | PTE_BIT_NX);
    }

    for (uintptr_t data_addr = data_start; data_addr < data_end; data_addr += PAGE_SIZE) {
        uintptr_t phys = data_addr - kernel_address->virtual_base + kernel_address->physical_base;
        vmm_map_page(kernel_page_map, data_addr, phys, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);
    }

/*     for(uint64_t base = 0x1000; base < 0x100000000 ; base += PAGE_SIZE){
        vmm_map_page(kernel_page_map, base, base, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX); // identity map everything in 32bit address space, maps madt and other tables
    } */

    vmm_set_ctx(kernel_page_map);

    asm volatile(
        "movq %%cr3, %%rax\n\
	   movq %%rax, %%cr3\n"
        : : : "rax"
   );
    
}

uint64_t *get_lower_table(uint64_t *page_map, uint64_t offset){

    if((page_map[offset] & PTE_BIT_PRESENT) != 0){
        //serial_kprintf("found\n");
        return (uint64_t*)( ((uint64_t)page_map[offset] & 0x000ffffffffff000) + hhdmoffset);
    }


    uint64_t *ret = pmm_alloc();

    if(!ret){
        klog(LOG_ERROR, __func__, "Failed to allocate page table");
        kprintf("page_map: 0x{xn}", (uint64_t)page_map);
        kprintf("offset: 0x{xn}", offset);
        
        return NULL;
    }

    memset((uint64_t*)((uint64_t)ret + hhdmoffset), 0, PAGE_SIZE);

    //serial_kprintf("page: 0x{x} at index: {d} = 0x{xn}", (uint64_t)page_map, offset, (uint64_t)ret);

    page_map[offset] = (uint64_t)ret | PTE_BIT_PRESENT | PTE_BIT_RW |  PTE_BIT_US;

    return (uint64_t*)( (uint64_t)ret + hhdmoffset );


}

void vmm_map_page(uint64_t *page_map, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags){

    uint64_t pml4_offset = (virt_addr >> 39) & 0x1ff;
    uint64_t pdp_offset = (virt_addr >> 30) & 0x1ff;
    uint64_t pd_offset = (virt_addr >> 21) & 0x1ff;
    uint64_t pt_offset = (virt_addr >> 12) & 0x1ff;
    //uint64_t pp_offset = virt_addr & 0x1ff;

    uint64_t *pdp = get_lower_table(page_map, pml4_offset);

    //erial_kprintf("pdp: 0x{xn}", (uint64_t)pdp);

    if(!pdp){
        klog(LOG_ERROR, __func__, "Failed to allocate PDP");
        kkill();
    }

    uint64_t *pd = get_lower_table(pdp, pdp_offset);

    if(!pd){
        klog(LOG_ERROR, __func__, "Failed to allocate PDP");
        kkill();
    }

    //serial_kprintf("pd: 0x{xn}", (uint64_t)pd);

    uint64_t *pt = get_lower_table(pd, pd_offset);

    if(!pt){
        klog(LOG_ERROR, __func__, "Failed to allocate PDP");
        kkill();
    }

    //serial_kprintf("pt: 0x{xn}", (uint64_t)pt);

/*     uint64_t *pp = get_lower_table(pt, pt_offset);

    if(!pp){
        klog(LOG_ERROR, __func__, "Failed to allocate PDP");
        kkill();
    } */

    pt[pt_offset] = phys_addr | flags;

    asm volatile(
        "movq %%cr3, %%rax\n\
	   movq %%rax, %%cr3\n"
        : : : "rax"
   );

}

void vmm_free_page(uint64_t *page_map, uint64_t virt_addr){
    uint64_t pml4_offset = (virt_addr >> 39) & 0x1ff;
    uint64_t pdp_offset = (virt_addr >> 30) & 0x1ff;
    uint64_t pd_offset = (virt_addr >> 21) & 0x1ff;
    uint64_t pt_offset = (virt_addr >> 12) & 0x1ff;

    uint64_t *pdp = get_lower_table(page_map, pml4_offset);


    if(!pdp){
        klog(LOG_ERROR, __func__, "Failed to allocate PDP");
        kkill();
    }

    uint64_t *pd = get_lower_table(pdp, pdp_offset);

    if(!pd){
        klog(LOG_ERROR, __func__, "Failed to allocate PDP");
        kkill();
    }


    uint64_t *pt = get_lower_table(pd, pd_offset);

    if(!pt){
        klog(LOG_ERROR, __func__, "Failed to allocate PDP");
        kkill();
    }

    /* Free the page at the physical address pointed by the pt entry */
    pmm_free((uint64_t*)(pt[pt_offset] & 0x000ffffffffff000));

    /* Set it to zero (mark as not present) */
    pt[pt_offset] = 0;

    asm volatile(
        "movq %%cr3, %%rax\n\
	    movq %%rax, %%cr3\n"
        : : : "rax"
   );
}

