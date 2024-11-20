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

    extern uint64_t kernel_end;

    /* Place the kernel heap at the end of the executables memory */
    kernel_heap_start = kernel_end + PAGE_SIZE;

    kprintf("kernel_heap_start: 0x{xn}", kernel_heap_start);

    int ret = vmm_map_continous_pages(kernel_heap_start, KERNEL_HEAP_SIZE / PAGE_SIZE + 1, PTE_BIT_PRESENT | PTE_BIT_RW | PTE_BIT_NX);

    if(ret == -1){
        klog(LOG_ERROR, __func__, "Failed to allocate kernel heap!\n");
        kkill();
    }
    for(uint64_t j = KERNEL_HEAP_SIZE / PAGE_SIZE; j > 0; j--){
        heap_free((uint64_t*)(kernel_heap_start + j * PAGE_SIZE));
        serial_kprintf("0x{xn}", (kernel_heap_start + j * PAGE_SIZE));
    }

    heap_free_page_count = KERNEL_HEAP_SIZE / PAGE_SIZE;

    kprintf("Free heap pages: {dn}", heap_free_page_count);

    kprintf("Done!\n");
}

/* Allocates n number of continous pages, returns the address of the first one */
void *kmalloc(uint64_t size){

    /* How many pages to allocate */
    uint64_t pages_to_alloc = ALIGN_UP(size, PAGE_SIZE) / PAGE_SIZE;

    kprintf("pages_to_alloc: {dn}", pages_to_alloc);

    /* Get the first page, which we will return to the callee */
    uint64_t *ret = heap_alloc();

    if(!ret){
        klog(LOG_ERROR, __func__, "kmalloc: Failed to allocate memory");
        kkill();
    }

    kprintf("ret: 0x{x}", (uint64_t)ret);

    /* Allocate the rest of the pages */
    for(uint64_t i = 0; i < pages_to_alloc; i++){
        if(!heap_alloc()){
            klog(LOG_ERROR, __func__, "kmalloc: Failed to allocate memory");
            kkill();
        }
    }

    return ret;
}

void kfree(void *addr){

    /* Determine how many pages this address had allocated */
    uint64_t size = (uint64_t)addr / (uint64_t)kernel_heap_start;
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
        kprintf("kmem: Heap is out of memory!\n");
        return NULL;
    }

    /* Fetch the address of the free page in free_list and make it point to the next free page */
    uint64_t *addr = heap_free_list;
    heap_free_list = (uint64_t*)(*heap_free_list);
    heap_free_page_count--;

    return addr;
}