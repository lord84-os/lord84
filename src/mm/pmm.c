#include <limine.h>
#include <stdio.h>
#include <SFB25.h>
#include <string.h>
#include <lock.h>
#include "pmm.h"
#include "kmalloc.h"

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};

extern uint64_t hhdmoffset;

uint64_t pmm_free_page_count = 0;
uint64_t pmm_page_count = 0;
uint64_t mem_size = 0;

struct limine_memmap_response *memmap_response;

/* Freelist implementation */
uint64_t *free_list = NULL;

atomic_flag pmm_lock = ATOMIC_FLAG_INIT;

void pmm_free(uint64_t *addr){
    acquire_lock(&pmm_lock);
    uint64_t *virt_addr = (uint64_t*)((uint64_t)addr+hhdmoffset);
    /* Make the given page point to the previous free page */
    
    *virt_addr = (uint64_t)free_list;

    /* Make the free_list point to the newly freed page */
    free_list = virt_addr;

    pmm_free_page_count++;
    free_lock(&pmm_lock);
    return;
}

uint64_t *pmm_alloc(){
    acquire_lock(&pmm_lock);
    if(pmm_free_page_count <= 0){
        return NULL;
    }

    /* Fetch the address of the free page in free_list and make it point to the next free page */
    uint64_t *addr = (uint64_t*)((uint64_t)free_list - hhdmoffset);
    free_list = (uint64_t*)(*free_list);
    pmm_free_page_count--;
    free_lock(&pmm_lock);
    return addr;
}

void pmm_init(){

    if(memmap_request.response == NULL){
        klog(LOG_ERROR, __func__, "Memmap response is null");
        kkill();
    }

    memmap_response = memmap_request.response;


    struct limine_memmap_entry **entries = memmap_response->entries;

    for(uint64_t i = 0; i < memmap_response->entry_count; i++){
        switch (entries[i]->type) {
            case LIMINE_MEMMAP_USABLE:
                //kprintf("usable: base: 0x{x}, length: 0x{xn}", entries[i]->base, entries[i]->length);
                mem_size += entries[i]->length;
                break;
            default:
                ;
                //kprintf("base: 0x{x}, length: 0x{xn}", entries[i]->base, entries[i]->length);
            
        }
    }

    kprintf("pmm: got a total of {d}MB of memory\n", mem_size / 1048576);


    bool first_entry = true;


    uint64_t j;
    uint64_t i;

    /* Dogshit fix this */
    for(i = 0; i < memmap_response->entry_count; i++){
        switch (entries[i]->type) {
            case LIMINE_MEMMAP_USABLE:
                /* First set the first entry if it isn't set already */
                if(first_entry == true){
                    first_entry = false;
                    free_list = (uint64_t*)(entries[i]->base + hhdmoffset);
                    j = 1;
                }else{
                    j = 0;
                }

                for(; j < (entries[i]->length / BLOCK_SIZE); j++){
                    pmm_free((uint64_t*)(entries[i]->base + j*BLOCK_SIZE));
                    pmm_page_count++;
                }
        }
    }

}