#include <stdio.h>
#include <string.h>
#include <SFB25.h>
#include "../hal/smp.h"
#include <error.h>
#include "../mm/kmalloc.h"
#include "sched.h"

extern context *save_context();

extern void switch_context(context *old, context *new);

#define QUANTUM_US 10000

int next_pid = 1;

void idle_task(){
    kprintf("Hello world from bruhd task!\n");
    for(;;);
}

void test_task(){
    kprintf("Hello world from scheduled task!\n");
    return;
}

/* Setup a process structure */
proc *alloc_process(void){
    asm("cli");

    cpu_state *state = get_cpu_struct();
    kprintf("hi10\n");
    proc *proc_list = state->process_list;

    uint8_t *sp;

    for(uint64_t i = 0; i < PROC_MAX; i++){
        if(proc_list[i].state == UNUSED){
            /* Set the process ready to be executed */
            kprintf("hi6\n");
            proc_list[i].state = READY;
            proc_list[i].kstack = kmalloc(INITIAL_STACK_SIZE);

            if(proc_list[i].kstack == NULL){
                klog(LOG_ERROR, __func__, "Failed to alloc stack");
            }
            kprintf("hi7\n");

            proc_list[i].pid = next_pid++;

            sp = (uint8_t*)((uint64_t)proc_list[i].kstack + INITIAL_STACK_SIZE);

            proc_list[i].context.rip = 0;
            proc_list[i].context.rsp = (uint64_t)sp;

            kprintf("hi8\n");

            asm("sti");

            return &proc_list[i];
        }
    }
    asm("sti");

    return NULL;
}

kstatus add_task(uint64_t *entry){
    proc *proc = alloc_process();

    if (proc == NULL) {
        klog(LOG_ERROR, __func__, "proc == null!");
        kkill();
    }

    proc->context.rip = (uint64_t)entry;

    return KERNEL_STATUS_SUCCESS;
}

void scheduler_init(){
    cpu_state *state = get_cpu_struct();

    if(state->current_process != NULL){
        kprintf("sched: scheduler on CPU {d} already initialized!\n", state->lapic_id);
        kkill();
    }
    kprintf("hi1\n");
    proc *proc_list = state->process_list;

    /* Put the idle task */
    proc idle = {0, 0, 0, READY, {0, 0, 0, 0, 0, 0, 0, 0, 0}};

    /* Make the idle structure the firstr process */
    proc_list[0] = idle;

    kprintf("hi2\n");
    add_task((uint64_t*)test_task);
    kprintf("hi5\n");

    for(;;){
        for(int i = 0; i < PROC_MAX; i++){
            if(proc_list[i].state == READY){
                
                context old_state = state->current_process->context;

                state->current_process = &proc_list[i];
                state->current_process->state = RUNNING;

                switch_context(&old_state, &state->current_process->context);

            }
        }
    }
}

void scheduler_tick(){
    cpu_state *state = get_cpu_struct();
    proc *proc_list = state->process_list;



}