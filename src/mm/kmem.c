#include <stdbool.h>
#include <stdint.h>
#include <limine.h>
#include <stdio.h>
#include <lord84.h>
#include <string.h>
#include "pmm.h"
#include "vmm.h"
#include "kmem.h"

#define KERNEL_HEAP_SIZE    0x100000

extern struct limine_memmap_response *memmap_response;

uint64_t kernel_heap_start = 0;
uint64_t kernel_heap_end = 0;
uint64_t kernel_heap_counter = 0;

/* Freelist implementation */
uint64_t *heap_free_list = NULL;

uint64_t heap_free_page_count = 0;

extern uint64_t hhdmoffset;

void kernel_heap_init(){

    bool suitable_place_found = false;

    struct limine_memmap_entry **entries = memmap_response->entries;

    /* Place the heap at the first usable address */
    for(uint64_t i = 0; i < memmap_response->entry_count; i++){
        if(entries[i]->type == LIMINE_MEMMAP_USABLE){

            if(entries[i]->length < KERNEL_HEAP_SIZE){
                continue;
            }

            suitable_place_found = true;
            kernel_heap_start = entries[i]->base;
            kernel_heap_end = kernel_heap_start + KERNEL_HEAP_SIZE;

            break;

        }
    }

    if(!suitable_place_found){
        klog(LOG_ERROR, __func__, "Failed to find a suitable place for the kernel heap");
    }

    /* Set the heap as used in the PMM --- WARNING - SUS */
    for(uint64_t j = 0; j < KERNEL_HEAP_SIZE; j++){
        pmm_alloc();
    }

    for(uint64_t j = 0; j < KERNEL_HEAP_SIZE; j += 4096){
        heap_free((uint64_t*)(kernel_heap_start + hhdmoffset + j));
    }

    kprintf("Free heap pages: {dn}", heap_free_page_count);

    kprintf("Done!\n");
}

/* Allocates n number of continous pages, returns the address of the first one */
void *kmalloc(uint64_t size){

    /* How many pages to allocate */
    uint64_t pages_to_alloc = size / PAGE_SIZE;

    /* Get the first page, which we will return to the callee */
    uint64_t *ret = heap_alloc();

    if(!ret){
        klog(LOG_ERROR, __func__, "kmalloc: Failed to allocate memory");
        kkill();
    }

    /* Allocate the rest of the pages */
    for(uint64_t i = 0; i < pages_to_alloc; i++){
        if(!heap_alloc()){
            klog(LOG_ERROR, __func__, "kmalloc: Failed to allocate memory");
            kkill();
        }
    }

    return ret;
}

void heap_free(uint64_t *addr){
    /* Make the given page point to the previous free page */
    *addr = (uint64_t)heap_free_list;

    /* Make the free_list point to the newly freed page */
    heap_free_list = addr;

    heap_free_page_count++;

    return;
}

uint64_t *heap_alloc(){

    if(heap_free_page_count <= 0){
        kprintf("Hey!\n");
        return NULL;
    }

    /* Fetch the address of the free page in free_list and make it point to the next free page */
    heap_free_list = (uint64_t*)(*heap_free_list);
    heap_free_page_count--;

    memset(heap_free_list, 0, 4096);

    return heap_free_list;
}