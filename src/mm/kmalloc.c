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

void *krealloc(void *addr, uint64_t size) {
    if (addr == NULL) {
        return kmalloc(size);
    }

    if (size == 0) {
        kfree(addr);
        return NULL;
    }

    // Find the block corresponding to the pointer
    int i;
    for (i = 0; i < KERNEL_MAX_BLOCK; i++) {
        if (base[i].addr == (uint64_t)addr) {
            break;
        }
    }

    if (i == KERNEL_MAX_BLOCK) {
        kprintf("krealloc: attempted to realloc non-heap address!\n");
        kkill();
    }

    block_t *block = &base[i];
    uint64_t old_size = block->size;

    // If the current size is already sufficient, return the same pointer
    if (old_size >= size) {
        return addr;
    }

    // Check if this block is the last allocated block in the array
    bool is_last = true;
    for (int j = i + 1; j < KERNEL_MAX_BLOCK; j++) {
        if (base[j].addr != 0) {
            is_last = false;
            break;
        }
    }

    // If it's the last block, check if there's enough space to expand
    if (is_last) {
        uint64_t current_end = block->addr + block->size;
        uint64_t heap_end = (uint64_t)heap_addr + KERNEL_HEAP_SIZE;
        uint64_t available = heap_end - current_end;

        if (available >= (size - old_size)) {
            // Expand the block in place
            block->size = size;
            return addr;
        }
    }

    // Allocate a new block, copy data, and free the old block
    void *new_ptr = kmalloc(size);
    if (!new_ptr) {
        return NULL; // Allocation failed
    }

    memcpy(new_ptr, addr, old_size);
    kfree(addr);

    return new_ptr;
}