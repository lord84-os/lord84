#include <stdbool.h>
#include <stdint.h>
#include <limine.h>
#include <stdio.h>
#include <lord84.h>
#include <string.h>
#include <lock.h>
#include "pmm.h"
#include "vmm.h"
#include "kmalloc.h"

#define KERNEL_MAX_BLOCK    512

typedef struct block_t {
    uint64_t addr;
    uint32_t size;
    bool free;
} block_t;

block_t *base = NULL;
uint64_t *heap_addr = NULL;

void kernel_heap_init(){
    extern struct limine_memmap_response *memmap_response;
    extern uint64_t pmm_page_count;

    /* Allocate memory for the blocks*/
    base = kernel_allocate_memory(KERNEL_MAX_BLOCK * sizeof(block_t), PTE_BIT_RW | PTE_BIT_NX);

    if(!base){
        klog(LOG_ERROR, __func__, "Failed to allocate memory for kernel heap blocks");
        kkill();
    }

    memset(base, 0, KERNEL_MAX_BLOCK * sizeof(block_t));

    /* Allocate memory for the heap */
    heap_addr = kernel_allocate_memory(KERNEL_HEAP_SIZE, PTE_BIT_RW | PTE_BIT_NX);

    if(!heap_addr){
        klog(LOG_ERROR, __func__, "Failed to allocate memory for the kernel heap");
        kkill();       
    }

    base->free = true;
}

void *kmalloc(uint64_t size){

    /* First check if there is a free block which fits the size requirement */
    for(int i = 0; i < KERNEL_MAX_BLOCK; i++){
        if(base[i].addr && base[i].free && base[i].size >= size){
            base[i].free = false;
            return (void*)base[i].addr;
        }
    }

    int i = 0;
    /* Parse the list until you find the next free block */
    while (!base[i].free && !base[i].addr){
        if(i > KERNEL_MAX_BLOCK){
            /* Over max block limit */
            return NULL;
        }

        i++;
    }

    /* Fill this block in */
    uint64_t addr = (uint64_t)heap_addr;
    
    /* Calculate address offset */
    for(int j = 0; j < i; j++){
        if(addr > ((uint64_t)heap_addr + KERNEL_HEAP_SIZE)){
            /* Out of heap memory */
            return NULL;
        }

        addr += base[j].size;
    }

    memset((uint64_t*)addr, 0, size);

    base[i].addr = addr;
    base[i].free = false;
    base[i].size = size;

    return (void*)addr;
}

void kfree(void *addr){
    for(int i = 0; i < KERNEL_MAX_BLOCK; i++){
        if(base[i].addr == (uint64_t)addr){
            base[i].free = true;
            memset((void*)base[i].addr, 0, base[i].size);
            return;
        }
    }

    kprintf("kfree: attempted to free non-heap address!\n");
    kkill();
}